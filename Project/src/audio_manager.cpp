#include "audio_manager.h"
#include <iostream>
#include <fstream>
#include <vector>

AudioManager::AudioManager() : isInitialized(false) {
    device = nullptr;
    context = nullptr;
    musicSource = 0;
    musicBuffer = 0;
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::init() {
    // Open default device
    device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open audio device" << std::endl;
        return false;
    }

    // Create context
    context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cerr << "Failed to create audio context" << std::endl;
        alcCloseDevice(device);
        return false;
    }

    // Make context current
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to make audio context current" << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        return false;
    }

    // Generate source and buffer
    alGenSources(1, &musicSource);
    alGenBuffers(1, &musicBuffer);

    // Set default source properties
    alSourcef(musicSource, AL_GAIN, 1.0f);
    alSourcei(musicSource, AL_LOOPING, AL_TRUE);

    isInitialized = true;
    return true;
}

bool AudioManager::loadAndPlayMusic(const std::string& filename, bool loop) {
    if (!isInitialized) {
        std::cerr << "Audio manager not initialized" << std::endl;
        return false;
    }

    // Stop any currently playing music
    stopMusic();

    // Open the audio file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio file: " << filename << std::endl;
        return false;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file data
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    // Load audio data into buffer
    alBufferData(musicBuffer, AL_FORMAT_STEREO16, buffer.data(), fileSize, 44100);

    // Attach buffer to source
    alSourcei(musicSource, AL_BUFFER, musicBuffer);

    // Set looping
    alSourcei(musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);

    // Play the source
    alSourcePlay(musicSource);

    return true;
}

void AudioManager::stopMusic() {
    if (!isInitialized) return;
    alSourceStop(musicSource);
}

void AudioManager::setVolume(float volume) {
    if (!isInitialized) return;

    // Clamp volume between 0.0 and 1.0
    volume = std::max(0.0f, std::min(1.0f, volume));
    alSourcef(musicSource, AL_GAIN, volume);
}

void AudioManager::cleanup() {
    if (!isInitialized) return;

    alDeleteSources(1, &musicSource);
    alDeleteBuffers(1, &musicBuffer);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    isInitialized = false;
} 