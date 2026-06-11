// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include "shared/log.hpp"

namespace Funkin::App {
    struct RecentProject {
        std::string name;
        std::string path;
        int64_t     lastOpened = 0;
    };

    struct Project {
    public:
        static Project& get();

        bool load(const std::filesystem::path& projectFile);
        bool create(const std::filesystem::path& folder, const std::string& name);

        bool        isLoaded()    const { return m_loaded; }
        bool        isFirstRun()  const { return m_firstRun; }
        void        unload()            { m_loaded = false; }

        const std::string&            getName()     const { return m_name; }
        const std::filesystem::path&  getRoot()     const { return m_root; }
        const std::filesystem::path&  getAssets()   const { return m_assets; }
        const std::filesystem::path&  getCacheDir() const { return m_cacheDir; }
        std::string getRootStr() const { return m_root.string(); }

        static std::vector<RecentProject> loadRecent();
        static void                       saveRecent(const std::vector<RecentProject>&);
        static void                       addRecent(const RecentProject&);
        static std::filesystem::path      recentPath();

    private:
        std::string           m_name;
        std::filesystem::path m_root;
        std::filesystem::path m_assets;
        std::filesystem::path m_cacheDir;
        bool                  m_loaded   = false;
        bool                  m_firstRun = false;
    };
}