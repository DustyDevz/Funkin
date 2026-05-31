// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <filesystem>

namespace Funkin {
   class Filesystem {
    public:
        static void init();
        
        static std::filesystem::path getLocalDir();
        static std::filesystem::path getModsDir();
        static std::filesystem::path getCacheDir();
        
        static std::filesystem::path resolve(const std::string& virtualPath);
        
        static std::string readString(const std::string& virtualPath);
        static bool writeString(const std::string& virtualPath, const std::string& content);

    private:
        static std::filesystem::path s_localPath;
        static std::filesystem::path s_modsPath;
        static std::filesystem::path s_cachePath;
    };
}