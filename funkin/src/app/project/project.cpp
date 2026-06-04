// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "project.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#endif

namespace Funkin::App {
    Project& Project::get() {
        static Project s;
        return s;
    }

    std::filesystem::path Project::recentPath() {
        #ifdef _WIN32
            PWSTR appData = nullptr;
            SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appData);
            std::filesystem::path base = appData;
            CoTaskMemFree(appData);
        #else
            std::filesystem::path base = std::filesystem::path(getenv("HOME")) / ".local/share";
        #endif
        return base / "FunkinEngine" / "recent.json";
    }

    std::vector<RecentProject> Project::loadRecent() {
        std::vector<RecentProject> result;
        auto path = recentPath();
        if (!std::filesystem::exists(path)) return result;

        try {
            std::ifstream f(path);
            auto j = nlohmann::json::parse(f);
            auto time = std::chrono::system_clock::now();
            for (auto& entry : j["recent"]) {
                RecentProject rp;
                rp.name       = entry["name"].get<std::string>();
                rp.path       = entry["path"].get<std::string>();
                rp.lastOpened = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
                result.push_back(rp);
            }
        } catch (...) {
            LOG_WARN("Failed to parse recent.json");
        }
        return result;
    }

    void Project::saveRecent(const std::vector<RecentProject>& recents) {
        auto path = recentPath();
        std::filesystem::create_directories(path.parent_path());

        nlohmann::json j;
        j["recent"] = nlohmann::json::array();
        for (auto& rp : recents) {
            j["recent"].push_back({ {"name", rp.name}, {"path", rp.path} });
        }

        std::ofstream f(path);
        f << j.dump(4);
    }

    void Project::addRecent(const RecentProject& rp) {
        auto recents = loadRecent();
        recents.erase(std::remove_if(recents.begin(), recents.end(),
            [&](const RecentProject& r) { return r.path == rp.path; }),
            recents.end());
        recents.insert(recents.begin(), rp);
        if (recents.size() > 10)
            recents.resize(10);
        saveRecent(recents);
    }

    bool Project::load(const std::filesystem::path& projectFile) {
        if (!std::filesystem::exists(projectFile)) {
            LOG_ERR("Project file not found: {}", projectFile.string());
            return false;
        }

        try {
            std::ifstream f(projectFile);
            auto j = nlohmann::json::parse(f);
            name    = j["name"].get<std::string>();
            root    = projectFile.parent_path();
            assets  = root / "bin" / "assets";
            m_loaded = true;
            LOG_PRINT("Project loaded: {}", name);
            LOG_PRINT("Project root: {}", root.string());
            addRecent({ name, root.string() });
            return true;
        } catch (const std::exception& e) {
            LOG_ERR("Failed to parse project.funkin: {}", e.what());
            return false;
        }
    }

    bool Project::create(const std::filesystem::path& folder,
                        const std::string& pName) {
        if (!std::filesystem::exists(folder)) {
            LOG_ERR("Folder does not exist: {}", folder.string());
            return false;
        }

        nlohmann::json j;
        j["name"]    = pName;
        j["assets"]  = "bin/assets";

        {
            std::ofstream f(folder / "project.funkin");
            f << j.dump(4);
        }

        auto base = folder / "bin" / "assets";
        auto dirs = {
            base / "images" / "ui",
            base / "images" / "characters",
            base / "images" / "stages",
            base / "audio"  / "music",
            base / "audio"  / "sfx",
            base / "audio"  / "songs",
            base / "video"  / "cutscenes",
            base / "fonts",
            base / "shaders" / "src",
            base / "shaders" / "compiled",
            base / "data"   / "charts",
            base / "data"   / "characters",
            base / "data"   / "stages",
        };

        for (auto& d : dirs)
            std::filesystem::create_directories(d);

        LOG_PRINT("Project created: {} at {}", pName, folder.string());
        return load(folder / "project.funkin");
    }
}