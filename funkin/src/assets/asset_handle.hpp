// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"
#include <memory>

namespace Funkin::Assets {
    template<typename T>
    class AssetHandle {
    public:
        AssetHandle() = default;
        AssetHandle(std::shared_ptr<T> ptr, AssetID id)
            : m_ptr(std::move(ptr)), m_id(id) {}

        T*       get()        const { return m_ptr.get(); }
        T&       operator*()  const { return *m_ptr; }
        T*       operator->() const { return m_ptr.get(); }
        bool     valid()      const { return m_ptr != nullptr; }
        AssetID  id()         const { return m_id; }
        explicit operator bool() const { return valid(); }
        void     reset()            { m_ptr.reset(); m_id = {}; }

    private:
        std::shared_ptr<T> m_ptr;
        AssetID            m_id;
    };

    using TextureHandle = AssetHandle<TextureAsset>;
    using AudioHandle   = AssetHandle<AudioAsset>;
    using FontHandle    = AssetHandle<FontAsset>;
    using ShaderHandle  = AssetHandle<ShaderAsset>;
    using BinaryHandle  = AssetHandle<BinaryAsset>;
}