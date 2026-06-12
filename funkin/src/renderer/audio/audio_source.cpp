// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "audio_source.hpp"
#include "assets/assets.hpp"
#include "audio_manager.hpp"
#include "shared/log.hpp"

namespace Funkin::Audio {

    AudioSource::~AudioSource() {
        destroyPipeline();
    }

    void AudioSource::load(const std::string& id, const std::string& group) {
        destroyPipeline();
        m_clip = Assets::AssetManager::get().load<Assets::AudioClip>(id, group);
        if (!m_clip) {
            LOG_WARN("Failed to load {}", id);
            return;
        }
        setupPipeline();
    }

    void AudioSource::loadAsync(const std::string& id, const std::string& group) {
        destroyPipeline();
        Assets::AssetManager::get().loadAsync<Assets::AudioClip>(id, group,
            [this](Assets::AudioHandle clip) {
                m_clip = clip;
                if (m_clip) setupPipeline();
            });
    }

    void AudioSource::play() {
        if (!m_initialized || !AudioManager::get().isReady()) return;

        ma_sound_seek_to_pcm_frame(&m_sound, 0);
        ma_sound_set_volume(&m_sound, volume);
        ma_sound_set_pitch(&m_sound, pitch);
        ma_sound_set_looping(&m_sound, m_loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(&m_sound);
    }

    void AudioSource::stop() {
        if (m_initialized) {
            ma_sound_stop(&m_sound);
            ma_sound_seek_to_pcm_frame(&m_sound, 0);
        }
    }

    void AudioSource::pause() {
        if (m_initialized) ma_sound_stop(&m_sound);
    }

    void AudioSource::resume() {
        if (m_initialized) ma_sound_start(&m_sound);
    }

    void AudioSource::setLoop(bool loop) {
        m_loop = loop;
        if (m_initialized) {
            ma_sound_set_looping(&m_sound, m_loop ? MA_TRUE : MA_FALSE);
        }
    }

    void AudioSource::setVolume(float v) {
        volume = v;
        if (m_initialized) ma_sound_set_volume(&m_sound, v);
    }

    void AudioSource::setPitch(float p) {
        pitch = p;
        if (m_initialized) ma_sound_set_pitch(&m_sound, p);
    }

    void AudioSource::seek(float seconds) {
        if (!m_initialized || !m_clip) return;
        ma_uint64 frame = static_cast<ma_uint64>(seconds * m_clip->sampleRate);
        ma_sound_seek_to_pcm_frame(&m_sound, frame);
    }

    float AudioSource::position() const {
        if (!m_initialized || !m_clip) return 0.f;
        ma_uint64 frame = 0;
        ma_sound_get_cursor_in_pcm_frames(const_cast<ma_sound*>(&m_sound), &frame);
        return static_cast<float>(frame) / m_clip->sampleRate;
    }

    float AudioSource::duration() const {
        if (!m_clip) return 0.f;
        return static_cast<float>(m_clip->frameCount) / m_clip->sampleRate;
    }

    bool AudioSource::isPlaying() const {
        return m_initialized && ma_sound_is_playing(const_cast<ma_sound*>(&m_sound));
    }

    bool AudioSource::hasClip() const {
        return m_clip != nullptr;
    }

    void AudioSource::setupPipeline() {
        if (m_initialized || !m_clip || m_clip->pcmData.empty()) return;
        if (!AudioManager::get().isReady()) return;

        ma_audio_buffer_config bufCfg = ma_audio_buffer_config_init(
            ma_format_f32,
            m_clip->channels,
            m_clip->frameCount,
            m_clip->pcmData.data(),
            nullptr
        );

        if (ma_audio_buffer_init(&bufCfg, &m_buffer) != MA_SUCCESS) {
            LOG_WARN("Buffer init failed for {}", m_clip->id);
            return;
        }

        if (ma_sound_init_from_data_source(
                AudioManager::get().engine(),
                &m_buffer,
                0,
                nullptr,
                &m_sound) != MA_SUCCESS) 
        {
            LOG_WARN("Sound init failed for {}", m_clip->id);
            ma_audio_buffer_uninit(&m_buffer);
            return;
        }

        m_initialized = true;
    }

    void AudioSource::destroyPipeline() {
        if (!m_initialized) return;

        ma_sound_uninit(&m_sound);
        ma_audio_buffer_uninit(&m_buffer);
        m_initialized = false;
    }

}