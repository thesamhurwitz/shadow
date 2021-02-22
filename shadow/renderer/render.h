#pragma once

#include <glm/glm.hpp>

#include "shadow/renderer/vertex_array.h"
#include "shadow/renderer/shader.h"
#include "shadow/renderer/camera.h"
#include "shadow/renderer/texture.h"

namespace Shadow {

class Render {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(Camera& camera);
    static void EndScene();

    static void SetClearColor(glm::vec4 const& color);
    static void Clear();

    static void DrawRect(glm::vec3 const& position, glm::vec2 const& size, glm::vec4 const& color);
    static void DrawRect(glm::vec3 const& position, glm::vec2 const& size, std::shared_ptr<Texture> const& texture);

private:
    struct SceneData {
        std::shared_ptr<Shader> colorShader;
        std::shared_ptr<Shader> textureShader;
        std::shared_ptr<VertexArray> rectVertexArray;
    };

    inline static SceneData* sceneData;
};

}