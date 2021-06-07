#include "scene.h"

#include "shadow/ecs/game_object.h"
#include "shadow/components/transform.h"
#include "shadow/components/sprite_component.h"
#include "shadow/components/script_component.h"
#include "shadow/renderer/render.h"
#include "shadow/events/application_events.h"
#include "shadow/components/camera_component.h"
#include "shadow/components/animator_component.h"

#include "shadow/events/application_events.h"


namespace Shadow {
Scene::Scene()
    : mRegistry(this) {


    On<WindowResizeEvent>([&](auto e) {
        for (auto& object : mRegistry.GetObjects()) {
            if (!object->HasComponent<CameraComponent>())
                continue;

            object->GetComponent<CameraComponent>().cameraController.Recalculate();
        }
    });
}

GameObject& Scene::Create() {
    return mRegistry.CreateObject();
}

void Scene::Destroy(GameObject& object) {
    mRegistry.RemoveObject(object);
}

void Scene::Load() {
    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<ScriptComponent>())
            continue;

        auto& c = object->GetComponent<ScriptComponent>();
        if (!c.loaded) {
            c.script->OnLoad();
            c.loaded = true;
        }
    }
}

void Scene::OnEnable() {
    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<ScriptComponent>())
            continue;

        object->GetComponent<ScriptComponent>().script->OnEnable();
    }
}

void Scene::OnDisable() {
    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<ScriptComponent>())
            continue;

        object->GetComponent<ScriptComponent>().script->OnDisable();
    }
}

void Scene::Destroy() {
    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<ScriptComponent>())
            continue;

        object->GetComponent<ScriptComponent>().script->OnDestroy(); // TODO move to object or sth
    }
}

void Scene::VariableUpdate(float delta) {
    if (mSceneCamera == nullptr)
        return;

    { // Script Variable Update
        for (auto& object : mRegistry.GetObjects()) {
            if (!object->HasComponent<ScriptComponent>())
                continue;

            auto& c = object->GetComponent<ScriptComponent>();
            if (!c.loaded) {
                c.script->OnLoad();
                c.loaded = true;
            }

            c.script->VariableUpdate(delta);
        }
    }

    { // Animator update
        for (auto& object : mRegistry.GetObjects()) {
            if (!object->HasComponent<AnimatorComponent>())
                continue;

            auto& c = object->GetComponent<AnimatorComponent>();
            c.animationController.Update(delta);

            if (!object->HasComponent<SpriteComponent>())
                return;
            object->GetComponent<SpriteComponent>().sprite.mTexture = c.animationController.GetAnimation().GetTexture();
        }
    }


    Render::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
    Render::Clear();
    Render::BeginScene(*mSceneCamera);

    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<SpriteComponent>())
            continue;

        auto& transform = object->GetComponent<Transform>();
        auto& sprite = object->GetComponent<SpriteComponent>().sprite;

        glm::vec4 box = { transform.Position.x, transform.Position.y,
                          transform.Position.x + transform.Scale.x, transform.Position.y + transform.Scale.y };

        Render::DrawRect(box, 0.0f, sprite.mTexture.GetTexture(), sprite.mTexture.GetTexCoords(),
                         sprite.mRotation, sprite.mFlipX, sprite.mFlipY, sprite.mTint);
    }

    Render::EndScene();
}

void Scene::FixedUpdate(float delta) {
    for (auto& object : mRegistry.GetObjects()) {
        if (!object->HasComponent<ScriptComponent>())
            continue;

        auto& c = object->GetComponent<ScriptComponent>();
        if (!c.loaded)
        {
            c.script->OnLoad();
            c.loaded = true;
        }

        c.script->FixedUpdate(delta);
    }
}
}