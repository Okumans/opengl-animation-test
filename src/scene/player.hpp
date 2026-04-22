#pragma once

#include "glm/fwd.hpp"
#include "resource/animation_manager.hpp"
#include "scene/humanoid_entity.hpp"
#include "utility/enum_map.hpp"
#include "utility/not_initialized.hpp"
#include <cassert>
#include <memory>

enum class PlayerAnimation { IDLE, WALKING, RUNNING };

class Player : public HumaniodEntity<PlayerAnimation> {
public:
  Player(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
         glm::vec3 scale = glm::vec3(1.0f),
         glm::vec3 rotation = glm::vec3(0.0f))
      : HumaniodEntity<PlayerAnimation>(model, pos, scale, rotation) {}

  void setup() override {
    AnimationManager::ensureInit();

    m_animations.set(PlayerAnimation::IDLE,
                     AnimationManager::copy(AnimationName::KASANE_TETO_IDLE));
    m_animations.set(
        PlayerAnimation::WALKING,
        AnimationManager::copy(AnimationName::KASANE_TETO_WALKING));
    m_animations.set(
        PlayerAnimation::RUNNING,
        AnimationManager::copy(AnimationName::KASANE_TETO_RUNNING));

    assert(m_animations.isInitialized());

    m_animator.init(m_animations.ensureInitialized()
                        .get_checked(PlayerAnimation::IDLE)
                        .get());

    HumaniodEntity<PlayerAnimation>::_setupAnimationDuration();
  }
};
