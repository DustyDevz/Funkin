// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include "asset_types.hpp"

namespace Funkin::Assets {
#ifdef FUNKIN_DEBUG
    using WatchCallback = std::function<void(const std::filesystem::path&)>;

    class AssetWatcher {
    public:
        static AssetWatcher& get();

        void init(const std::filesystem::path& watchFolder,
                  std::chrono::milliseconds pollInterval = std::chrono::milliseconds(500));
        void shutdown();
        void onChanged(WatchCallback cb) { m_callback = std::move(cb); }
        void tick();

    private:
        AssetWatcher() = default;
        void watchLoop();

        struct FileState {
            uint64_t mtime    = 0;
            uint64_t fileSize = 0;
        };

        std::filesystem::path                            m_folder;
        std::chrono::milliseconds                        m_interval;
        std::unordered_map<std::string, FileState>       m_known;
        WatchCallback                                    m_callback;

        std::thread             m_thread;
        std::atomic<bool>       m_running{ false };

        std::vector<std::filesystem::path> m_pendingChanges;
        std::mutex                         m_changeMutex;
    };

#endif
}