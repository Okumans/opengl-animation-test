#pragma once

#include "glm/fwd.hpp"
#include "graphics/animation_state.hpp"
#include "resource/animation_manager.hpp"
#include "scene/humanoid_entity.hpp"
#include "utility/enum_map.hpp"
#include "utility/not_initialized.hpp"
#include "utility/random.hpp"
#include <cassert>
#include <memory>
#include <print>

enum class EnemyAnimation { IDLE, WALKING };

class Enemy : public HumaniodEntity<EnemyAnimation> {
private:
  AnimationState<float> m_waitingState;
  bool m_canWalk = false;

public:
  Enemy(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f), glm::vec3 rotation = glm::vec3(0.0f))
      : HumaniodEntity<EnemyAnimation>(model, pos, scale, rotation) {}

  void setup() override {
    AnimationManager::ensureInit();

    m_animations.set(EnemyAnimation::IDLE,
                     AnimationManager::copy(AnimationName::HATSUNE_MIKU_IDLE));
    m_animations.set(
        EnemyAnimation::WALKING,
        AnimationManager::copy(AnimationName::HATSUNE_MIKU_WALKING));

    assert(m_animations.isInitialized());

    m_animator.init(m_animations.ensureInitialized()
                        .get_checked(EnemyAnimation::IDLE)
                        .get());

    _setupAnimationDuration();
  }

  virtual void update(double delta_time) override {
    if (!m_rotateState.animationStarted && !m_positionState.animationStarted &&
        !m_waitingState.animationStarted) {

      float waiting_time = Random::randFloat(0.5f, 5.0f);
      m_waitingState.duration.ensureInitialized() = waiting_time;
      m_waitingState.startAnimation(0.0f, waiting_time);
    }

    if (!m_rotateState.animationStarted && !m_positionState.animationStarted &&
        m_canWalk) {
      float x = Random::randFloat(-2.0f, 2.0f);
      float z = Random::randFloat(-2.0f, 2.0f);

      moveWithAnimation({x, 0, z});

      if (Random::randChance(.25f))
        m_canWalk = false;
    }

    _updateWaitingAnimationState(delta_time);
    HumaniodEntity<EnemyAnimation>::update(delta_time);
  }

  virtual void _updateWaitingAnimationState(double delta_time) {
    if (!m_waitingState.animationStarted)
      return;

    m_waitingState.updateTimer(delta_time);
    float t = m_waitingState.getProgress();

    if (t == 1.0) {
      m_waitingState.animationStarted = false;
      m_canWalk = true;
    }
  }

  inline virtual void _setupAnimationDuration() override {
    m_positionState.duration.init(0.8f);
    m_rotateState.duration.init(0.6f);
    m_waitingState.duration.init(0.0f);
  }
};
