// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <miniaudio.h>
#include "assets/assets_types.hpp"
#include "shared/log.hpp"

namespace Funkin::Audio {
    class AudioManager {
    public:
        static AudioManager& get();

        bool init();
        void shutdown();
        void play(const Assets::AudioHandle& clip, float volume = 1.0f, float pitch = 1.0f);

        ma_engine* engine() { return &m_engine; }
        bool isReady() const { return m_ready; }

    private:
        AudioManager() = default;

        ma_engine m_engine;
        bool      m_ready = false;
    };
}