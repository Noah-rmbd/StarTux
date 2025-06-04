#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>
#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Initialize audio system
    bool init();
    
    // Load and play background music
    bool loadAndPlayMusic(const std::string& filename, bool loop = true);
    
    // Stop current music
    void stopMusic();
    
    // Set volume (0.0 to 1.0)
    void setVolume(float volume);
    
    // Clean up resources
    void cleanup();

private:
    ALCdevice* device;
    ALCcontext* context;
    ALuint musicSource;
    ALuint musicBuffer;
    bool isInitialized;
};

#endif // AUDIO_MANAGER_H 
