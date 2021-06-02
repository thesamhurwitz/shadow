#pragma once

#include "shadow/renderer/texture.h"
#include "shadow/renderer/sub_texture.h"

namespace Shadow {
class TextureAtlas {
public:
    struct AtlasTile {
        glm::vec2 offset;
        glm::vec2 size;
    };

    TextureAtlas(Ref<Texture> const& texture, std::initializer_list<AtlasTile> tiles);

    const SubTexture& operator[](size_t i) {
        return mTiles[i];
    }

private:
    std::vector<SubTexture> mTiles{};
};
}
