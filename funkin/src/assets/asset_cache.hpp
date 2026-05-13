// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"

namespace Funkin::Assets {
    template<typename T>
    class AssetCache {
    public:
        explicit AssetCache(size_t maxBytes) : m_maxBytes(maxBytes) {}

        void insert(AssetID id, std::shared_ptr<T> asset, size_t sizeBytes) {
            std::lock_guard lock(m_mutex);
            auto it = m_map.find(id);
            if (it != m_map.end()) {
                m_usedBytes -= it->second.sizeBytes;
                m_lru.erase(it->second.lruIt);
                m_map.erase(it);
            }
            evictIfNeeded(sizeBytes);
            m_lru.push_front(id);
            m_map[id] = { std::move(asset), m_lru.begin(), sizeBytes };
            m_usedBytes += sizeBytes;
        }

        std::shared_ptr<T> get(AssetID id) {
            std::lock_guard lock(m_mutex);
            auto it = m_map.find(id);
            if (it == m_map.end()) return nullptr;
            m_lru.splice(m_lru.begin(), m_lru, it->second.lruIt);
            return it->second.asset;
        }

        bool contains(AssetID id) const {
            std::lock_guard lock(m_mutex);
            return m_map.count(id) > 0;
        }

        void evict(AssetID id) {
            std::lock_guard lock(m_mutex);
            auto it = m_map.find(id);
            if (it == m_map.end()) return;
            m_usedBytes -= it->second.sizeBytes;
            m_lru.erase(it->second.lruIt);
            m_map.erase(it);
        }

        void clear() {
            std::lock_guard lock(m_mutex);
            m_map.clear();
            m_lru.clear();
            m_usedBytes = 0;
        }

        size_t usedBytes() const { std::lock_guard lock(m_mutex); return m_usedBytes; }
        size_t maxBytes()  const { return m_maxBytes; }
        size_t count()     const { std::lock_guard lock(m_mutex); return m_map.size(); }

    private:
        struct Entry {
            std::shared_ptr<T>                    asset;
            typename std::list<AssetID>::iterator lruIt;
            size_t                                sizeBytes;
        };

        void evictIfNeeded(size_t needed) {
            while (!m_lru.empty() && m_usedBytes + needed > m_maxBytes) {
                auto last = std::prev(m_lru.end());
                auto it   = m_map.find(*last);
                if (it != m_map.end()) {
                    m_usedBytes -= it->second.sizeBytes;
                    m_map.erase(it);
                }
                m_lru.erase(last);
            }
        }

        mutable std::mutex                               m_mutex;
        std::unordered_map<AssetID, Entry, AssetIDHasher> m_map;
        std::list<AssetID>                               m_lru;
        size_t                                           m_maxBytes  = 0;
        size_t                                           m_usedBytes = 0;
    };
}