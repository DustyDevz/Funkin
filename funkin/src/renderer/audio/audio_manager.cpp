// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "audio_manager.hpp"
#include "assets/assets_types.hpp"

namespace Funkin::Audio {
    AudioManager& AudioManager::get() {
        static AudioManager s;
        return s;
    }

    bool AudioManager::init() {
        ma_engine_config cfg = ma_engine_config_init();
        cfg.sampleRate = 44100;

        if (ma_engine_init(&cfg, &m_engine) != MA_SUCCESS) {
            LOG_ERR("Failed to init ma_engine");
            return false;
        }

        m_ready = true;
        LOG_PRINT("Initialized ({}Hz)", ma_engine_get_sample_rate(&m_engine));
        return true;
    }

    void AudioManager::shutdown() {
        if (!m_ready) return;
        ma_engine_uninit(&m_engine);
        m_ready = false;
        LOG_PRINT("Shutdown");
    }

    void AudioManager::play(const Assets::AudioHandle& clip, float volume, float pitch) {
        if (!m_ready || !clip || clip->pcmData.empty()) return;

        ma_sound* sound = new ma_sound{};

        ma_audio_buffer_config bufCfg = ma_audio_buffer_config_init(
            ma_format_f32,
            clip->channels,
            clip->frameCount,
            clip->pcmData.data(),
            nullptr
        );

        ma_audio_buffer* buf = new ma_audio_buffer{};
        if (ma_audio_buffer_init(&bufCfg, buf) != MA_SUCCESS) {
            LOG_WARN("Failed to init buffer for {}", clip->id);
            delete sound;
            delete buf;
            return;
        }

        if (ma_sound_init_from_data_source(&m_engine, buf, 0, nullptr, sound) != MA_SUCCESS) {
            LOG_WARN("Failed to init sound for {}", clip->id);
            ma_audio_buffer_uninit(buf);
            delete sound;
            delete buf;
            return;
        }

        ma_sound_set_volume(sound, volume);
        ma_sound_set_pitch(sound, pitch);
        ma_sound_set_end_callback(sound, [](void* userData, ma_sound* s) {
            auto* buf = reinterpret_cast<ma_audio_buffer*>(userData);
            ma_sound_uninit(s);
            ma_audio_buffer_uninit(buf);
            delete s;
            delete buf;
        }, buf);

        ma_sound_start(sound);
    }
}