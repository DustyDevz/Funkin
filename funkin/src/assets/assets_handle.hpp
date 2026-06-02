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
        AssetHandle() = default;
        explicit AssetHandle(std::shared_ptr<T> ptr) : m_ptr(std::move(ptr)) {}
        AssetHandle(std::nullptr_t) : m_ptr(nullptr) {}

        AssetHandle(const AssetHandle&)            = default;
        AssetHandle& operator=(const AssetHandle&) = default;
        AssetHandle(AssetHandle&&)                 = default;
        AssetHandle& operator=(AssetHandle&&)      = default;

        AssetHandle& operator=(std::nullptr_t) {
            m_ptr = nullptr;
            return *this;
        }

        AssetHandle& operator=(std::shared_ptr<T> ptr) {
            m_ptr = std::move(ptr);
            return *this;
        }

        T* get()        const { return m_ptr.get(); }
        T* operator->() const { return m_ptr.get(); }
        T& operator*()  const { return *m_ptr; }

        bool isValid()           const { return m_ptr != nullptr; }
        explicit operator bool() const { return isValid(); }
        long useCount()          const { return m_ptr.use_count(); }

        bool operator==(const AssetHandle& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const AssetHandle& other) const { return m_ptr != other.m_ptr; }

        const std::string& id() const {
            static std::string empty;
            return m_ptr ? m_ptr->id : empty;
        }

    private:
        std::shared_ptr<T> m_ptr;
    };

    template<typename T>
    AssetHandle<T> NullHandle() { return AssetHandle<T>{ nullptr }; }
}