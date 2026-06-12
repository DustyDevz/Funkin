// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "audio_loader.hpp"

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

namespace Funkin::Assets::Loaders {
    std::shared_ptr<AudioClip> loadAudio(
        const std::filesystem::path& path,
        const std::string& id,
        const std::string& group) {
        ma_decoder_config cfg = ma_decoder_config_init(
            ma_format_f32, 2, 44100);

        ma_decoder decoder;
        ma_result result = ma_decoder_init_file(
            path.string().c_str(), &cfg, &decoder);

        if (result != MA_SUCCESS) {
            LOG_ERR("Audio load failed: {} — {}", id, ma_result_description(result));
            return nullptr;
        }

        ma_uint64 frameCount = 0;
        ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

        if (frameCount == 0) {
            LOG_WARN("Audio has no frames: {}", id);
            ma_decoder_uninit(&decoder);
            return nullptr;
        }

        uint32_t channels   = 2;
        uint32_t sampleRate = 44100;
        size_t   byteCount  = (size_t)(frameCount * channels * sizeof(float));

        auto asset           = std::make_shared<AudioClip>();
        asset->id            = id;
        asset->group         = group;
        asset->sampleRate    = sampleRate;
        asset->channels      = channels;
        asset->frameCount    = frameCount;
        asset->sizeBytes     = byteCount;
        asset->pcmData.resize(byteCount);

        ma_uint64 framesRead = 0;
        ma_decoder_read_pcm_frames(&decoder,
            asset->pcmData.data(), frameCount, &framesRead);

        ma_decoder_uninit(&decoder);

        if (framesRead == 0) {
            LOG_ERR("Audio decode failed: {}", id);
            return nullptr;
        }

        asset->loaded = true;
        LOG_PRINT("Audio loaded: {} ({} frames, {:.2f}s)",
            id, framesRead, (double)framesRead / sampleRate);
        return asset;
    }
}