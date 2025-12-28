#pragma once
#include "engine/types.hpp"
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include "miniaudio.h"

class Audio {
friend class ClientEngine;

private:
    struct AudioInstance {
        ma_sound* sound;
        int maxInstances;
        std::vector<ma_sound*> instances;
    };

    ma_engine* engine;
    std::unordered_map<audio_id, AudioInstance> audioMap;
    audio_id nextAudioId;

    Audio();
    ~Audio();

public:
    audio_id create(std::string filepath, int maxInstances = 5);
    void play(audio_id id, glm::vec3 position, float volume = 1.0f);
    void destroy(audio_id id);
    void setListenerPosition(const glm::vec3 &position);
    void setListenerRotation(const glm::quat &rotation);
};