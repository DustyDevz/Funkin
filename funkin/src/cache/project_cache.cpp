// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "project_cache.hpp"
#include "app/project/project.hpp"

#include <fstream>
#include <thread>
#include <vector>
#include <format>

#include <nlohmann/json.hpp>
#include <xxhash.h>
#include <stb_image.h>
#include <lz4.h>

#include "shared/log.hpp"

namespace Funkin::Cache {
    static constexpr uint8_t  CACHE_MAGIC[4] = { 'F','K','T','X' };
    static constexpr uint8_t  CACHE_VERSION  = 2;

    ProjectCache& ProjectCache::get() {
        static ProjectCache s;
        return s;
    }

    uint64_t ProjectCache::hashFile(const std::filesystem::path& path) const {
        std::error_code ec;
        auto time = std::filesystem::last_write_time(path, ec);
        if (ec) return 0;
        
        auto size = std::filesystem::file_size(path, ec);
        if (ec) return 0;

        std::string pathStr = path.string();
        size_t timeValue = time.time_since_epoch().count();

        std::vector<uint8_t> metaBuffer(pathStr.size() + sizeof(size) + sizeof(timeValue));
        uint8_t* ptr = metaBuffer.data();

        std::memcpy(ptr, pathStr.data(), pathStr.size()); ptr += pathStr.size();
        std::memcpy(ptr, &size, sizeof(size));            ptr += sizeof(size);
        std::memcpy(ptr, &timeValue, sizeof(timeValue));

        return XXH3_64bits(metaBuffer.data(), metaBuffer.size());
    }

    std::filesystem::path ProjectCache::getCachedPath(const std::filesystem::path& srcPath) const {
        uint64_t hash = hashFile(srcPath);
        if (hash == 0) return {};

        auto ktx = Funkin::App::Project::get().getCacheDir() / "cache" / "textures"
                   / std::format("{:x}.fktx", hash);

        return std::filesystem::exists(ktx) ? ktx : std::filesystem::path{};
    }

    void ProjectCache::warmAsync(
        const std::filesystem::path& assetsRoot,
        const std::filesystem::path& cacheDir,
        std::function<void()> onComplete)
    {
        m_progress.completed = 0;
        m_progress.total     = 0;
        m_progress.done      = false;
        m_progress.failed    = false;

        std::thread(&ProjectCache::warmWorker, this,
            assetsRoot, cacheDir, std::move(onComplete)).detach();
    }

    std::filesystem::path ProjectCache::getThumbPath(const std::filesystem::path& srcPath) const {
        uint64_t hash = hashFile(srcPath);
        if (hash == 0) return {};
        
        auto thumb = Funkin::App::Project::get().getCacheDir() / "cache" / "textures"
                    / std::format("{:x}.thumb", hash);
                    
        return std::filesystem::exists(thumb) ? thumb : std::filesystem::path{};
    }

    void ProjectCache::warmWorker(
        const std::filesystem::path& assetsRoot,
        const std::filesystem::path& cacheDir,
        std::function<void()> onComplete)
    {
        auto textureDir   = cacheDir / "cache" / "textures";
        auto manifestPath = cacheDir / "cache.json";

        std::error_code ec;
        std::filesystem::create_directories(textureDir, ec);
        if (ec) {
            LOG_ERR("failed to create {}: {}", textureDir.string(), ec.message());
            m_progress.done = true;
            if (onComplete) onComplete();
            return;
        }

        auto imagesRoot = assetsRoot / "images";
        if (!std::filesystem::exists(imagesRoot)) {
            LOG_WARN("Images dir not found: {}", imagesRoot.string());
            m_progress.done = true;
            if (onComplete) onComplete();
            return;
        }

        std::vector<std::filesystem::path> images;
        for (auto& entry : std::filesystem::recursive_directory_iterator(imagesRoot)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            for (auto& c : ext) c = (char)tolower(c);
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                images.push_back(entry.path());
        }

        m_progress.total = (int)images.size();
        LOG_PRINT("Warming {} images -> {}", images.size(), textureDir.string());

        nlohmann::json manifest = nlohmann::json::object();
        if (std::filesystem::exists(manifestPath)) {
            try {
                std::ifstream mf(manifestPath);
                manifest = nlohmann::json::parse(mf);
            } catch (...) {
                LOG_WARN("Bad cache.json, rebuilding");
            }
        }

        for (auto& imgPath : images) {
            {
                std::lock_guard lk(m_progress.mutex);
                m_progress.currentFile = imgPath.filename().string();
            }

            uint64_t hash = hashFile(imgPath);
            if (hash == 0) {
                LOG_WARN("Hash failed: {}", imgPath.string());
                m_progress.completed++;
                continue;
            }

            std::string hashStr = std::format("{:x}", hash);
            auto outPath = textureDir / (hashStr + ".fktx");
            auto thumbPath = textureDir / (hashStr + ".thumb");

            if (std::filesystem::exists(outPath) && std::filesystem::exists(thumbPath) && manifest.contains(hashStr)) {
                m_progress.completed++;
                continue;
            }

            int w, h, ch;
            uint8_t* px = stbi_load(imgPath.string().c_str(), &w, &h, &ch, 4);
            if (!px) {
                LOG_WARN("Stb_image failed: {} ({})",
                    imgPath.filename().string(), stbi_failure_reason());
                m_progress.completed++;
                continue;
            }

            bool wrote = false;
            {
                int rawSize = w * h * 4;
                int maxCompressed = LZ4_compressBound(rawSize);
                std::vector<char> compressed(maxCompressed);

                int compressedSize = LZ4_compress_default(
                    reinterpret_cast<const char*>(px),
                    compressed.data(),
                    rawSize,
                    maxCompressed
                );

                if (compressedSize > 0) {
                    std::ofstream out(outPath, std::ios::binary);
                    if (out) {
                        out.write(reinterpret_cast<const char*>(CACHE_MAGIC), 4);
                        out.write(reinterpret_cast<const char*>(&CACHE_VERSION), 1);
                        out.write(reinterpret_cast<const char*>(&w),              4);
                        out.write(reinterpret_cast<const char*>(&h),              4);
                        out.write(reinterpret_cast<const char*>(&rawSize),        4);
                        out.write(reinterpret_cast<const char*>(&compressedSize), 4);
                        out.write(compressed.data(), compressedSize);
                        wrote = out.good();
                    }
                }
            }

            if (!wrote) {
                LOG_WARN("Write failed for {}", outPath.string());
                std::filesystem::remove(outPath);
                stbi_image_free(px);
                m_progress.completed++;
                continue;
            }

            {
                int tw, th;
                float aspect = (float)w / (float)h;
                if (aspect > (200.f / 110.f)) { tw = 200; th = (int)(200.f / aspect); }
                else                           { th = 110; tw = (int)(110.f * aspect); }
                tw = std::max(tw, 1);
                th = std::max(th, 1);

                std::vector<uint8_t> thumbBuf(tw * th * 4);
                for (int ty = 0; ty < th; ty++) {
                    for (int tx = 0; tx < tw; tx++) {
                        int sx = std::min((int)((float)tx / tw * w), w - 1);
                        int sy = std::min((int)((float)ty / th * h), h - 1);
                        int si = (sy * w + sx) * 4;
                        int di = (ty * tw + tx) * 4;
                        thumbBuf[di+0] = px[si+0];
                        thumbBuf[di+1] = px[si+1];
                        thumbBuf[di+2] = px[si+2];
                        thumbBuf[di+3] = px[si+3];
                    }
                }

                std::ofstream tout(thumbPath, std::ios::binary);
                if (tout) {
                    tout.write(reinterpret_cast<const char*>(&tw), 4);
                    tout.write(reinterpret_cast<const char*>(&th), 4);
                    tout.write(reinterpret_cast<const char*>(thumbBuf.data()), thumbBuf.size());
                }
            }

            stbi_image_free(px);
            LOG_PRINT("Cached {} ({}x{})", imgPath.filename().string(), w, h);

            manifest[hashStr] = {
                { "file", imgPath.filename().string() },
                { "path", imgPath.string()            },
                { "w",    w                           },
                { "h",    h                           }
            };

            m_progress.completed++;
        }

        writeManifest(cacheDir, manifest);
        LOG_PRINT("Warm complete");
        m_progress.done = true;
        if (onComplete) onComplete();
    }

    void ProjectCache::writeManifest(
        const std::filesystem::path& cacheDir,
        const nlohmann::json& manifest)
    {
        auto manifestPath = cacheDir / "cache.json";
        std::ofstream out(manifestPath);
        if (out)
            out << manifest.dump(2);
        else
            LOG_ERR("Failed to write cache.json");
    }
}