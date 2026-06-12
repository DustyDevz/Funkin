// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <string>
#include <miniaudio.h>
#include "assets/assets_types.hpp"

namespace Funkin::Audio {

    class AudioSource {
    public:
        float volume = 1.0f;
        float pitch  = 1.0f;

        AudioSource() = default;
        ~AudioSource();

        void load(const std::string& id, const std::string& group = "");
        void loadAsync(const std::string& id, const std::string& group = "");
        
        void play();
        void stop();
        void pause();
        void resume();

        void setVolume(float v);
        void setPitch(float p);
        void setLoop(bool loop);
        void seek(float seconds);

        float position() const;
        float duration() const;
        bool isPlaying() const;
        bool hasClip() const;
        bool isLooping() const { return m_loop; }

    private:
        Assets::AudioHandle m_clip        = nullptr;
        ma_sound            m_sound;
        ma_audio_buffer     m_buffer;
        bool                m_loop        = false;
        bool                m_initialized = false;

        void setupPipeline();
        void destroyPipeline();
    };

}