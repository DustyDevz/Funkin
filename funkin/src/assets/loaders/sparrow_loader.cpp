// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "sparrow_loader.hpp"
#include "texture_loader.hpp"

namespace Funkin::Assets::Loaders {
    static std::string attr(const std::string& line, const std::string& key) {
        auto pos = line.find(key + "=\"");
        if (pos == std::string::npos) return "";
        pos += key.size() + 2;
        auto end = line.find('"', pos);
        return line.substr(pos, end - pos);
    }

    static float attrF(const std::string& line, const std::string& key, float def = 0.0f) {
        auto v = attr(line, key);
        return v.empty() ? def : std::stof(v);
    }

    std::shared_ptr<SparrowAtlas> loadSparrow(
        const std::filesystem::path& xmlPath,
        const std::string& id,
        const std::string& group) {

        std::ifstream f(xmlPath);
        if (!f.is_open()) {
            LOG_ERR("Sparrow XML not found: {}", id);
            return nullptr;
        }

        auto atlas      = std::make_shared<SparrowAtlas>();
        atlas->id       = id;
        atlas->group    = group;
        
        std::string line;
        std::string imageName;
        while (std::getline(f, line)) {
            if (line.find("TextureAtlas") != std::string::npos) {
                imageName = attr(line, "imagePath");
                break;
            }
        }

        if (imageName.empty()) {
            LOG_ERR("Sparrow XML missing imagePath: {}", id);
            return nullptr;
        }

        auto texPath = xmlPath.parent_path() / imageName;
        auto texId   = id.substr(0, id.find_last_of('.')) + ".png";
        atlas->texture = AssetHandle<Texture>(loadTexture(texPath, texId, group));
        if (!atlas->texture) {
            LOG_ERR("Sparrow texture not found: {}", texPath.string());
            return nullptr;
        }

        while (std::getline(f, line)) {
            if (line.find("SubTexture") == std::string::npos) continue;

            SparrowFrame frame;
            frame.name   = attr(line, "name");
            frame.x      = attrF(line, "x");
            frame.y      = attrF(line, "y");
            frame.w      = attrF(line, "width");
            frame.h      = attrF(line, "height");
            frame.frameX = attrF(line, "frameX", 0.0f);
            frame.frameY = attrF(line, "frameY", 0.0f);
            frame.frameW = attrF(line, "frameWidth",  frame.w);
            frame.frameH = attrF(line, "frameHeight", frame.h);

            atlas->frameIndex[frame.name] = atlas->frames.size();
            atlas->frames.push_back(frame);
        }

        atlas->sizeBytes = atlas->frames.size() * sizeof(SparrowFrame);
        atlas->loaded    = !atlas->frames.empty();
        LOG_PRINT("Sparrow loaded: {} ({} frames)", id, atlas->frames.size());
        return atlas;
    }
}