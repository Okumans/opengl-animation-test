#pragma once

#include "scene/game_object.hpp"

#include <glm/fwd.hpp>
#include <memory>

class WorldObject : public GameObject {
public:
  WorldObject(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
              glm::vec3 scale = glm::vec3(1.0f),
              glm::vec3 rotation = glm::vec3(0.0f))
      : GameObject(model, pos, scale, rotation, false) {}

  [[nodiscard]] GameObjectType getObjectType() const override {
    return GameObjectType::WORLD;
  }
};
