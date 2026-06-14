// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "filesystem.hpp"

#include <fstream>
#include <sstream>
#include <windows.h>
#include <shlobj.h>
#include <QFile>

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

    bool Filesystem::isEngineResource(const std::string& virtualPath) {
        return virtualPath.rfind("engine://", 0) == 0;
    }

    std::filesystem::path Filesystem::resolve(const std::string& virtualPath) {
        if (virtualPath.rfind("local://", 0) == 0)
            return s_localPath / virtualPath.substr(8);
        if (virtualPath.rfind("mods://", 0) == 0)
            return s_modsPath / virtualPath.substr(7);
        if (virtualPath.rfind("cache://", 0) == 0)
            return s_cachePath / virtualPath.substr(8);
        if (virtualPath.rfind("engine://", 0) == 0) {
            return {};
        }
        return std::filesystem::path(virtualPath);
    }

    std::vector<uint8_t> Filesystem::readBytesFromQtResource(const std::string& qtPath) {
        QFile file(QString::fromStdString(qtPath));
        if (!file.open(QIODevice::ReadOnly)) return {};
        auto data = file.readAll();
        return std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(data.constData()),
            reinterpret_cast<const uint8_t*>(data.constData()) + data.size()
        );
    }

    std::vector<uint8_t> Filesystem::readBytes(const std::string& virtualPath) {
        if (virtualPath.rfind("engine://", 0) == 0) {
            std::string qtPath = ":/" + virtualPath.substr(9);
            qtPath = ":/engine/" + virtualPath.substr(9);
            return readBytesFromQtResource(qtPath);
        }

        auto physPath = resolve(virtualPath);
        if (physPath.empty() || !std::filesystem::exists(physPath)) return {};

        std::ifstream f(physPath, std::ios::binary);
        if (!f) return {};
        return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>()
        );
    }

    std::string Filesystem::readString(const std::string& virtualPath) {
        if (virtualPath.rfind("engine://", 0) == 0) {
            auto bytes = readBytes(virtualPath);
            return std::string(bytes.begin(), bytes.end());
        }

        auto physPath = resolve(virtualPath);
        if (physPath.empty() || !std::filesystem::exists(physPath)) return "";

        std::ifstream file(physPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool Filesystem::writeString(const std::string& virtualPath, const std::string& content) {
        auto physPath = resolve(virtualPath);
        if (physPath.empty()) return false;

        std::filesystem::create_directories(physPath.parent_path());
        std::ofstream file(physPath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return false;
        file << content;
        return true;
    }
}