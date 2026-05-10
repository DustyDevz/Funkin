// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "asset_watcher.hpp"

#ifdef FUNKIN_DEBUG

#include "asset_compiler.hpp"
#include "asset_packer.hpp"
#include "asset_loader.hpp"
#include <stdexcept>

namespace Funkin::Assets {

    AssetWatcher& AssetWatcher::get() {
        static AssetWatcher s;
        return s;
    }

    void AssetWatcher::init(const std::filesystem::path& watchFolder,
                             std::chrono::milliseconds pollInterval) {
        m_folder   = watchFolder;
        m_interval = pollInterval;

        std::error_code ec;
        for (auto& entry : std::filesystem::recursive_directory_iterator(watchFolder, ec)) {
            if (!entry.is_regular_file()) continue;
            auto& p = entry.path();
            FileState fs;
            auto lwt   = std::filesystem::last_write_time(p, ec);
            fs.mtime   = ec ? 0 : (uint64_t)lwt.time_since_epoch().count();
            fs.fileSize= ec ? 0 : (uint64_t)std::filesystem::file_size(p, ec);
            m_known[p.string()] = fs;
        }

        m_running = true;
        m_thread  = std::thread([this]{ watchLoop(); });
        printf("[AssetWatcher] Watching: %s (poll every %lldms)\n",
               watchFolder.string().c_str(), (long long)pollInterval.count());
    }

    void AssetWatcher::shutdown() {
        m_running = false;
        if (m_thread.joinable()) m_thread.join();
    }

    void AssetWatcher::watchLoop() {
        while (m_running) {
            std::this_thread::sleep_for(m_interval);

            std::error_code ec;
            std::vector<std::filesystem::path> changed;

            std::unordered_map<std::string, FileState> current;
            for (auto& entry : std::filesystem::recursive_directory_iterator(m_folder, ec)) {
                if (!entry.is_regular_file()) continue;
                auto& p = entry.path();
                auto  ext = p.extension().string();
                if (ext == ".funkin" || ext == ".fkpak") continue;

                FileState fs;
                auto lwt   = std::filesystem::last_write_time(p, ec);
                fs.mtime   = ec ? 0 : (uint64_t)lwt.time_since_epoch().count();
                fs.fileSize= ec ? 0 : (uint64_t)std::filesystem::file_size(p, ec);
                current[p.string()] = fs;

                auto it = m_known.find(p.string());
                if (it == m_known.end()) {
                    changed.push_back(p);
                } else if (it->second.mtime != fs.mtime || it->second.fileSize != fs.fileSize) {
                    changed.push_back(p);
                }
            }

            m_known = std::move(current);

            if (!changed.empty()) {
                std::lock_guard lock(m_changeMutex);
                for (auto& p : changed)
                    m_pendingChanges.push_back(p);
            }
        }
    }

    void AssetWatcher::tick() {
        std::vector<std::filesystem::path> changes;
        {
            std::lock_guard lock(m_changeMutex);
            changes.swap(m_pendingChanges);
        }

        if (changes.empty()) return;

        printf("[AssetWatcher] %zu file(s) changed, hot-reloading...\n", changes.size());

        if (m_callback) {
            for (auto& p : changes)
                m_callback(p);
        }
    }
}

#endif