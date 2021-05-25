#pragma once

#include "shadow/renderer/texture.h"

namespace Shadow {
class SubTexture {
public:
    explicit SubTexture(Ref<Texture> texture);
    SubTexture(Ref<Texture> texture, const glm::vec4& texCoords);
    SubTexture(Ref<Texture> texture, const glm::vec2& size, const glm::vec2& offset);

    Ref<Texture> GetTexture() const { return mTexture; }
    const glm::vec2* GetTexCoords() const { return mTexCoords; }

private:
    Ref<Texture> mTexture;
    glm::vec2 mTexCoords[4]{};
};
}