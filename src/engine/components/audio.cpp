#include "engine/components/audio.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include "spdlog/spdlog.h"

Audio::Audio() : nextAudioId(1) {
    engine = new ma_engine();
    ma_result result = ma_engine_init(NULL, engine);
    if (result != MA_SUCCESS) {
        spdlog::error("Audio::Audio: Failed to initialize miniaudio engine");
        return;
    }
    spdlog::trace("Audio::Audio: Initialized miniaudio engine");
}

Audio::~Audio() {
    for (auto& pair : audioMap) {
        AudioInstance& instance = pair.second;
        for (auto* sound : instance.instances) {
            ma_sound_uninit(sound);
            delete sound;
        }
        ma_sound_uninit(instance.sound);
        delete instance.sound;
    }
    audioMap.clear();
    ma_engine_uninit(engine);
    delete engine;
}

audio_id Audio::create(std::string filepath, int maxInstances) {
    audio_id id = nextAudioId++;
    
    AudioInstance audioInstance;
    audioInstance.maxInstances = maxInstances;
    audioInstance.sound = new ma_sound();
    
    ma_result result = ma_sound_init_from_file(engine, filepath.c_str(), 0, NULL, NULL, audioInstance.sound);
    if (result != MA_SUCCESS) {
        spdlog::error("Audio::create: Failed to load audio file: {}", filepath);
        delete audioInstance.sound;
        return 0;
    }
    
    // Pre-allocate sound instances for pooling
    for (int i = 0; i < maxInstances; ++i) {
        ma_sound* sound = new ma_sound();
        ma_sound_init_from_file(engine, filepath.c_str(), 0, NULL, NULL, sound);
        ma_sound_set_looping(sound, MA_FALSE);
        ma_sound_stop(sound);
        ma_sound_seek_to_pcm_frame(sound, 0);
        audioInstance.instances.push_back(sound);
    }
    
    audioMap[id] = audioInstance;
    spdlog::trace("Audio::create: Created audio with id {} from file: {}", id, filepath);
    
    return id;
}

void Audio::play(audio_id id, glm::vec3 position, float volume) {
    auto it = audioMap.find(id);
    if (it == audioMap.end()) {
        spdlog::error("Audio::play: Invalid audio_id {}", id);
        return;
    }
    
    AudioInstance& audioInstance = it->second;
    
    // Find a stopped sound instance to reuse
    ma_sound* soundToPlay = nullptr;
    for (auto* sound : audioInstance.instances) {
        if (!ma_sound_is_playing(sound)) {
            soundToPlay = sound;
            break;
        }
    }
    
    if (soundToPlay == nullptr) {
        spdlog::warn("Audio::play: No available sound instances for audio_id {}", id);
        return;
    }

    ma_sound_stop(soundToPlay);
    ma_sound_seek_to_pcm_frame(soundToPlay, 0);
    
    // Set position and volume
    ma_sound_set_position(soundToPlay, position.x, position.y, position.z);
    ma_sound_set_volume(soundToPlay, volume);
    
    // Start playing
    ma_sound_start(soundToPlay);
}

void Audio::destroy(audio_id id) {
    auto it = audioMap.find(id);
    if (it == audioMap.end()) {
        spdlog::error("Audio::destroy: Invalid audio_id {}", id);
        return;
    }
    
    AudioInstance& audioInstance = it->second;
    for (auto* sound : audioInstance.instances) {
        ma_sound_uninit(sound);
        delete sound;
    }
    audioInstance.instances.clear();
    
    ma_sound_uninit(audioInstance.sound);
    delete audioInstance.sound;
    audioMap.erase(it);
    
    spdlog::trace("Audio::destroy: Destroyed audio with id {}", id);
}

void Audio::setListenerPosition(const glm::vec3 &position) {
    ma_engine_listener_set_position(engine, 0, position.x, position.y, position.z);
}

void Audio::setListenerRotation(const glm::quat &rotation) {
    // miniaudio uses forward and up vectors for listener orientation
    glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    
    ma_engine_listener_set_direction(engine, 0, forward.x, forward.y, forward.z);
}

