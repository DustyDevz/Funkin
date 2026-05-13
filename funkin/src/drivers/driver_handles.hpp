// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Renderer::GAL {
    template<typename Tag>
    struct Handle {
        uint32_t id = 0;
        bool valid() const { return id != 0; }
        bool operator==(const Handle& o) const { return id == o.id; }
        bool operator!=(const Handle& o) const { return id != o.id; }
    };

    struct TextureTag    {};
    struct BufferTag     {};
    struct ShaderTag     {};
    struct PipelineTag   {};
    struct SamplerTag    {};

    using TextureHandle  = Handle<TextureTag>;
    using BufferHandle   = Handle<BufferTag>;
    using ShaderHandle   = Handle<ShaderTag>;
    using PipelineHandle = Handle<PipelineTag>;
    using SamplerHandle  = Handle<SamplerTag>;
}

namespace std {
    template<typename Tag>
    struct hash<Funkin::Renderer::GAL::Handle<Tag>> {
        size_t operator()(const Funkin::Renderer::GAL::Handle<Tag>& h) const {
            return std::hash<uint32_t>{}(h.id);
        }
    };
}
