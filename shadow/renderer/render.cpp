#include "render.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <shadow/application/game_loop.h>

namespace Shadow {

struct RectVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoords;
    float TexIndex;
    float TilingFactor;
};

struct RenderData {
    static inline const uint32_t MaxRects = 10000;
    static inline const uint32_t MaxVertices = MaxRects * 4;
    static inline const uint32_t MaxIndices = MaxRects * 6;
    static inline const uint32_t MaxTextureSlots = 16;

    Ref<Shader> DefaultShader;

    Ref<VertexArray> RectVA;
    RectVertex* RectVertexBuffer = nullptr;

    std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;

    uint32_t RectCount = 0;
    uint32_t TextureSlot = 1;
    RectVertex* RectVertexBufferPtr = nullptr;
};

static RenderData renderData;


void Render::Init() {
    // OpenGL Options
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    // Preparing rect vertex data
    renderData.RectVA = MakeRef<VertexArray>();

    auto rectVB = MakeRef<VertexBuffer>(RenderData::MaxVertices * sizeof(RectVertex));
    rectVB->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoords" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_TilingFactor" },
    });
    renderData.RectVA->AddVertexBuffer(rectVB);

    renderData.RectVertexBuffer = new RectVertex[RenderData::MaxVertices];
    renderData.RectVertexBufferPtr = renderData.RectVertexBuffer;

    // Generating indices for rects
    auto* rectIndices = new uint32_t[+RenderData::MaxIndices];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < RenderData::MaxIndices; i += 6) {
        rectIndices[i + 0] = offset + 0;
        rectIndices[i + 1] = offset + 1;
        rectIndices[i + 2] = offset + 2;
        rectIndices[i + 3] = offset + 2;
        rectIndices[i + 4] = offset + 3;
        rectIndices[i + 5] = offset + 0;

        offset += 4;
    }
    auto rectIB = MakeRef<IndexBuffer>(rectIndices, RenderData::MaxIndices);
    renderData.RectVA->SetIndexBuffer(rectIB);
    delete[] rectIndices;

    // Compiling Default shader
    renderData.DefaultShader = MakeRef<Shader>("assets/shaders/Default.glsl");
    renderData.DefaultShader->Bind();

    // Uploading sampler2d array data to Default shader
    int32_t samplers[RenderData::MaxTextureSlots];
    for (int i = 0; i < RenderData::MaxTextureSlots; i++)
        samplers[i] = i;

    renderData.DefaultShader->UploadUniformIntArray("u_Textures", samplers, RenderData::MaxTextureSlots);

    // Generating 1x1 white texture for colored rects
    renderData.TextureSlots[0] = Texture::CreateWhiteTexture();

#ifdef SH_DEBUGGER
    Debugger::Stats.MaxRects = RenderData::MaxRects;
    Debugger::Stats.MaxTextureSlots = RenderData::MaxTextureSlots;
#endif
}

void Render::Shutdown() {
    delete[] renderData.RectVertexBuffer;
}

void Render::BeginScene(Camera& camera) {
    // Uploading ViewProjection matrix to Default shader
    renderData.DefaultShader->Bind();
    renderData.DefaultShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
}

void Render::EndScene() {
    Flush();
}

void Render::Flush() {
    // Binding all textures to corresponding texture slots
    for (uint32_t i = 0; i < renderData.TextureSlot; i++) {
        renderData.TextureSlots[i]->Bind(i);
    }

    // Uploading vertex data to GPU
    renderData.RectVA->GetVertexBuffers()[0]->SetData((const void*)renderData.RectVertexBuffer,
                                                      (renderData.RectVertexBufferPtr - renderData.RectVertexBuffer) * sizeof(RectVertex));

    renderData.RectVA->Bind();
    glDrawElements(GL_TRIANGLES, (int)renderData.RectCount * 6, GL_UNSIGNED_INT, nullptr);

#ifdef SH_DEBUGGER
    Debugger::Stats.DrawCalls++;
    Debugger::Stats.RectCount += renderData.RectCount;
    Debugger::Stats.TextureCount = renderData.TextureSlot;
#endif

    // Resetting iterators
    renderData.RectVertexBufferPtr = renderData.RectVertexBuffer;
    renderData.RectCount = 0;
    renderData.TextureSlot = 1;
}

void Render::DrawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, float rotation) {
    if (renderData.RectCount >= RenderData::MaxRects)
        Flush();

    const float texIndex = 0.0f;
    const float tilingFactor = 1.0f;

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(-rotation), { 0.0f, 0.0f, 1.0f }) *
            glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f});

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = {0.0f, 0.0f };
    renderData.RectVertexBufferPtr->TexIndex = texIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{0.5f, -0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = {1.0f, 0.0f };
    renderData.RectVertexBufferPtr->TexIndex = texIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{0.5f, 0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = {1.0f, 1.0f };
    renderData.RectVertexBufferPtr->TexIndex = texIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = {0.0f, 1.0f };
    renderData.RectVertexBufferPtr->TexIndex = texIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectCount++;
}

void Render::DrawRect(glm::vec3 const& position, glm::vec2 const& size, Ref<SubTexture> const& subTexture, float rotation) {
    auto texture = subTexture->GetTexture();

    if (renderData.RectCount >= RenderData::MaxRects)
        Flush();

    constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    const float tilingFactor = 1.0f;

    // Looking up the texture, if not found, add it to TextureSlots
    // If TextureSlots is full, Flushes the batch
    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < renderData.TextureSlot; i++) {
        if (*renderData.TextureSlots[i].get() == *texture.get()) {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f) {
        if (renderData.TextureSlot >= RenderData::MaxTextureSlots)
            Flush();

        textureIndex = (float)renderData.TextureSlot;
        renderData.TextureSlots[renderData.TextureSlot] = texture;
        renderData.TextureSlot++;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(-rotation), { 0.0f, 0.0f, 1.0f }) *
                          glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f});

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = subTexture->GetTexCoords()[0];
    renderData.RectVertexBufferPtr->TexIndex = textureIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{0.5f, -0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = subTexture->GetTexCoords()[1];
    renderData.RectVertexBufferPtr->TexIndex = textureIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{0.5f, 0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = subTexture->GetTexCoords()[2];
    renderData.RectVertexBufferPtr->TexIndex = textureIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectVertexBufferPtr->Position = transform * glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f };
    renderData.RectVertexBufferPtr->Color = color;
    renderData.RectVertexBufferPtr->TexCoords = subTexture->GetTexCoords()[3];
    renderData.RectVertexBufferPtr->TexIndex = textureIndex;
    renderData.RectVertexBufferPtr->TilingFactor = tilingFactor;
    renderData.RectVertexBufferPtr++;

    renderData.RectCount++;
}

void Render::DrawLine(const glm::vec2 &from, const glm::vec2 &to, float width, glm::vec4 color) {
    glm::vec3 pos = { (from.x + to.x) / 2, (from.y + to.y) / 2, 0.0f };
    glm::vec2 size = { fabs(from.x - to.x) + width, fabs(from.y - to.y) + width };

    DrawRect(pos, size, color);
}

void Render::SetClearColor(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void Render::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


}