// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <memory>
#include <string>
#include <cstdint>

namespace Funkin::Assets {
    struct AssetBase {
        std::string id;    // 'images/ui/test.png'
        std::string group; // 'gameplay', 'ui', '' = persistent
        uint64_t    sizeBytes = 0;
        bool        loaded = false;

        virtual ~AssetBase() = default;
    };

    template<typename T>
    class AssetHandle {
    public:
        virtual ~AssetHandle() = default;
        explicit AssetHandle(std::shared_ptr<T> ptr) : m_ptr(std::move(ptr)) {}

        T* get()        const { return m_ptr.get(); }
        T* operator->() const { return m_ptr.get(); }
        T& operator*()  const { return *m_ptr; }

        bool isValid()           const { return m_ptr != nullptr; }
        explicit operator bool() const { return isValid(); }
        long useCount()          const { return m_ptr.use_count(); }

        const std::string& id() const {
            static std::string empty;
            return m_ptr ? m_ptr->id : empty;
        }

    private:
        std::shared_ptr<T> m_ptr;
    };

    // fallback
    template<typename T>
    AssetHandle<T> NullHandle() { return AssetHandle<T>{nullptr}; }
}