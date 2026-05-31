// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "filesystem.hpp"

namespace Funkin {
    std::filesystem::path Filesystem::s_localPath;
    std::filesystem::path Filesystem::s_modsPath;
    std::filesystem::path Filesystem::s_cachePath;

    void Filesystem::init() {
        wchar_t localAppDataW[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataW))) {
            s_localPath = std::filesystem::path(localAppDataW) / "FunkinEngine";
            std::filesystem::create_directories(s_localPath);
            std::filesystem::create_directories(s_localPath / "logs");
            
            s_cachePath = s_localPath / "cache";
            std::filesystem::create_directories(s_cachePath);
        }

        wchar_t exeW[MAX_PATH];
        GetModuleFileNameW(NULL, exeW, MAX_PATH);
        s_modsPath = std::filesystem::path(exeW).parent_path() / "mods";
        std::filesystem::create_directories(s_modsPath);
    }

    std::filesystem::path Filesystem::getLocalDir() { return s_localPath; }
    std::filesystem::path Filesystem::getModsDir()  { return s_modsPath; }
    std::filesystem::path Filesystem::getCacheDir() { return s_cachePath; }

    std::filesystem::path Filesystem::resolve(const std::string& virtualPath) {
        if (virtualPath.rfind("local://", 0) == 0) {
            return s_localPath / virtualPath.substr(8);
        }
        if (virtualPath.rfind("mods://", 0) == 0) {
            return s_modsPath / virtualPath.substr(7);
        }
        if (virtualPath.rfind("cache://", 0) == 0) {
            return s_cachePath / virtualPath.substr(8);
        }
        return std::filesystem::path(virtualPath);
    }

    std::string Filesystem::readString(const std::string& virtualPath) {
        std::filesystem::path physicalPath = resolve(virtualPath);
        if (!std::filesystem::exists(physicalPath)) return "";

        std::ifstream file(physicalPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool Filesystem::writeString(const std::string& virtualPath, const std::string& content) {
        std::filesystem::path physicalPath = resolve(virtualPath);
        
        std::filesystem::path parentDir = physicalPath.parent_path();
        if (!std::filesystem::exists(parentDir)) {
            std::filesystem::create_directories(parentDir);
        }

        std::ofstream file(physicalPath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return false;

        file << content;
        return true;
    }
}