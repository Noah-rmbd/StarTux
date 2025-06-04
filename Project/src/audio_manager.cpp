#include "audio_manager.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <cstring> // Pour memcpy
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>
// Structure pour l'en-tête WAV
struct WavHeader {
  char riff[4];
  uint32_t fileSize;
  char wave[4];
  char fmt[4];
  uint32_t fmtSize;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char data[4];
  uint32_t dataSize;
};

AudioManager::AudioManager()
    : device(nullptr), context(nullptr), musicSource(0), musicBuffer(0),
      isInitialized(false) {}

AudioManager::~AudioManager() { cleanup(); }

bool AudioManager::init() {
  device = alcOpenDevice(nullptr);
  if (!device) {
    std::cerr << "Échec d'ouverture du périphérique audio" << std::endl;
    return false;
  }

  // Configuration du contexte
  ALCint attrs[] = {
      ALC_FREQUENCY,
      44100, // Force 44.1kHz
      ALC_MONO_SOURCES,
      1,
      ALC_STEREO_SOURCES,
      1,
      0 // Fin de liste
  };
  context = alcCreateContext(device, attrs);

  // Créer le contexte
  context = alcCreateContext(device, nullptr);
  if (!context) {
    std::cerr << "Échec de la création du contexte audio" << std::endl;
    alcCloseDevice(device);
    return false;
  }

  // Activer le contexte
  if (!alcMakeContextCurrent(context)) {
    std::cerr << "Échec de l'activation du contexte audio" << std::endl;
    alcDestroyContext(context);
    alcCloseDevice(device);
    return false;
  }

  // Générer les sources et buffers
  alGenSources(1, &musicSource);
  alGenBuffers(1, &musicBuffer);

  // Vérifier les erreurs
  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "Erreur lors de l'initialisation OpenAL" << std::endl;
    cleanup();
    return false;
  }

  isInitialized = true;
  return true;
}

bool AudioManager::loadAndPlayMusic(const std::string &filename, bool loop) {
  if (!isInitialized) {
    std::cerr << "AudioManager not initialized" << std::endl;
    return false;
  }

  // Réinitialisation sécurisée
  stopMusic();
  alSourcei(musicSource, AL_BUFFER, 0); // Détache tout buffer existant
  ALenum perror = alGetError();
  if (perror != AL_NO_ERROR) {
    std::cerr << "OpenAL reset error: " << perror << std::endl;
  }
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  size_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  // 1. Lire l'en-tête complet
  std::vector<char> header(44);
  if (!file.read(header.data(), 44)) {
    std::cerr << "Failed to read WAV header" << std::endl;
    return false;
  }

  // 2. Vérification de base du format WAV
  if (std::string(header.data(), 4) != "RIFF" ||
      std::string(header.data() + 8, 4) != "WAVE") {
    std::cerr << "Invalid WAV file format" << std::endl;
    return false;
  }

  // 3. Recherche du chunk 'data' dans les 1000 premiers bytes
  const size_t searchLimit = std::min<size_t>(1000, fileSize);
  std::vector<char> searchBuffer(searchLimit);
  file.seekg(0, std::ios::beg);
  if (!file.read(searchBuffer.data(), searchLimit)) {
    std::cerr << "Failed to read search buffer" << std::endl;
    return false;
  }

  size_t dataPos = 0;
  for (size_t i = 0; i < searchLimit - 4; i++) {
    if (std::string(searchBuffer.data() + i, 4) == "data") {
      dataPos = i;
      break;
    }
  }

  if (dataPos == 0) {
    std::cerr << "No 'data' chunk found in first " << searchLimit << " bytes"
              << std::endl;
    return false;
  }

  // 4. Lecture de la taille des données
  uint32_t dataSize;
  memcpy(&dataSize, searchBuffer.data() + dataPos + 4, 4);
  size_t dataStart = dataPos + 8;

  // 5. Validation des tailles
  if (dataStart + dataSize > fileSize) {
    std::cerr << "Invalid data size: " << dataSize << " (file only has "
              << fileSize - dataStart << " bytes remaining)" << std::endl;
    return false;
  }

  // 6. Lecture des données audio
  std::vector<char> audioData(dataSize);
  file.seekg(dataStart, std::ios::beg);
  if (!file.read(audioData.data(), dataSize)) {
    std::cerr << "Failed to read audio data" << std::endl;
    return false;
  }

  // 7. Configuration OpenAL
  alBufferData(musicBuffer, AL_FORMAT_STEREO16, audioData.data(),
               static_cast<ALsizei>(dataSize), 44100);

  ALenum error = alGetError();
  if (error != AL_NO_ERROR) {
    std::cerr << "OpenAL Error: ";
    switch (error) {
    case AL_INVALID_ENUM:
      std::cerr << "Format non supporté";
      break;
    case AL_INVALID_VALUE:
      break;
    case AL_OUT_OF_MEMORY:
      std::cerr << "Mémoire insuffisante";
      break;
    default:
      std::cerr << "Code " << error;
    }
    std::cerr << std::endl;
    return false;
  }

  alSourcei(musicSource, AL_BUFFER, static_cast<ALint>(musicBuffer));
  alSourcei(musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  alSourcePlay(musicSource);

  return alGetError() == AL_NO_ERROR;
}

void AudioManager::stopMusic() {
  if (isInitialized && musicSource) {
    alSourceStop(musicSource);
  }
}

void AudioManager::setVolume(float volume) {
  if (isInitialized && musicSource) {
    volume = std::max(0.0f, std::min(1.0f, volume)); // Clamper entre 0 et 1
    alSourcef(musicSource, AL_GAIN, volume);
  }
}

void AudioManager::cleanup() {
  if (!isInitialized)
    return;

  stopMusic();

  if (musicSource) {
    alDeleteSources(1, &musicSource);
    musicSource = 0;
  }

  if (musicBuffer) {
    alDeleteBuffers(1, &musicBuffer);
    musicBuffer = 0;
  }

  if (context) {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    context = nullptr;
  }

  if (device) {
    alcCloseDevice(device);
    device = nullptr;
  }

  isInitialized = false;
}
void AudioManager::debugSoundSystem() {
  std::cout << "=== Debug Audio System ===" << std::endl;
  std::cout << "Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER)
            << std::endl;
  std::cout << "Context: " << (alcGetCurrentContext() ? "Actif" : "Inactif")
            << std::endl;

  ALint sourceState;
  alGetSourcei(musicSource, AL_SOURCE_STATE, &sourceState);
  std::cout << "Source state: ";
  switch (sourceState) {
  case AL_INITIAL:
    std::cout << "INITIAL";
    break;
  case AL_PLAYING:
    std::cout << "PLAYING";
    break;
  case AL_PAUSED:
    std::cout << "PAUSED";
    break;
  case AL_STOPPED:
    std::cout << "STOPPED";
    break;
  default:
    std::cout << "UNKNOWN";
  }
  std::cout << std::endl;
}
