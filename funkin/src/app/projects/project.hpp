// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "shared/log.hpp"

namespace Funkin::App {
    struct RecentProject {
        std::string name;
        std::string path;
    };

    struct Project {
        std::string                  name;
        std::string                  author;
        std::string                  version;
        std::filesystem::path        root;
        std::filesystem::path        assets;

        static Project& get();

        bool load(const std::filesystem::path& projectFile);
        bool create(const std::filesystem::path& folder,
                    const std::string& name,
                    const std::string& author,
                    const std::string& version);

        bool        isLoaded() const { return m_loaded; }
        void        unload()         { m_loaded = false; }
        
        static std::vector<RecentProject> loadRecent();
        static void                        saveRecent(const std::vector<RecentProject>&);
        static void                        addRecent(const RecentProject&);
        static std::filesystem::path       recentPath();

    private:
        bool m_loaded = false;
    };
}