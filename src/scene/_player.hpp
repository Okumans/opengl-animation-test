#pragma once

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/animation.hpp"
#include "graphics/animation_state.hpp"
#include "graphics/animator.hpp"
#include "resource/animation_manager.hpp"
#include "scene/game_object.hpp"
#include "utility/enum_map.hpp"
#include "utility/not_initialized.hpp"
#include "utility/utility.hpp"
#include <cassert>
#include <memory>

enum class [[deprecated(
    "Use PlayerAnimation from player.hpp instead")]] PlayerAnimation {
  IDLE,
  WALKING,
  RUNNING
};

class [[deprecated("Use Player which inherited from Entity instead")]] Player
    : public GameObject {
private:
  SettableNotInitialized<
      EnumMap<PlayerAnimation, std::shared_ptr<Animation>>, "m_animations",
      EnumMapValidator<EnumMap<PlayerAnimation, std::shared_ptr<Animation>>>>
      m_animations;
  NotInitialized<Animator, "m_animator"> m_animator;
  PlayerAnimation m_playingAnimation = PlayerAnimation::IDLE;

  AnimationState<float> m_rotateState;
  AnimationState<glm::vec3> m_positionState;

public:
  Player(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
         glm::vec3 scale = glm::vec3(1.0f),
         glm::vec3 rotation = glm::vec3(0.0f))
      : GameObject(model, pos, scale, rotation) {
    m_positionState.duration.init(0.3f);
    m_rotateState.duration.init(0.2f);
  }

  void setup() {
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
  }

  void moveWithAnimation(glm::vec3 vec) {
    if (glm::length(vec) < 0.001f)
      return;

    glm::vec3 new_target_pos = m_position + vec;
    float target_yaw = glm::degrees(std::atan2(vec.x, vec.z));

    _setAnimation(PlayerAnimation::WALKING);

    m_positionState.startAnimation(m_position, new_target_pos);
    m_rotateState.startAnimation(m_rotation.y, target_yaw);

    m_positionState.timer = 0;
    m_rotateState.timer = 0;
  }

  void move(glm::vec3 vec) {
    m_rotateState.reset();
    m_positionState.reset();

    m_position += vec;
    m_rotation = glm::normalize(m_position);
  }

  virtual void update(double delta_time) override {
    _updateRotateAnimationState(delta_time);
    _updatePositionAnimationState(delta_time);

    if (m_positionState.animationStarted == false &&
        m_rotateState.animationStarted == false) {
      _setAnimation(PlayerAnimation::IDLE);
    }

    _updateAnimation(delta_time);
  }

  void draw(const RenderContext &ctx) override {
    if (!m_model)
      return;

    ctx.shader.setBool("u_HasAnimation", true);

    _updateTransform();
    ctx.shader.setMat4("u_Model", m_modelMatrix);
    m_animator.ensureInitialized().apply(ctx.shader);

    m_model->draw(ctx);

    ctx.shader.setBool("u_HasAnimation", false);
  }

private:
  void _setAnimation(PlayerAnimation animation) {
    if (animation != m_playingAnimation) {
      m_playingAnimation = animation;
      float blend_duration =
          (m_playingAnimation == PlayerAnimation::WALKING) ? 0.12f : 0.18f;
      m_animator.ensureInitialized().playAnimation(
          m_animations.ensureInitialized()
              .get_checked(m_playingAnimation)
              .get(),
          blend_duration);
    }
  }

  void _updateRotateAnimationState(double delta_time) {
    if (!m_rotateState.animationStarted)
      return;

    m_rotateState.updateTimer((float)delta_time);
    float t = m_rotateState.getProgress();

    float current_yaw = lerpAngle(m_rotateState.start, m_rotateState.target, t);

    setRotation({0.0f, current_yaw, 0.0f});

    if (t >= 1.0f) {
      m_rotateState.animationStarted = false;
      m_rotation.y = std::fmod(m_rotation.y, 360.0f);
    }
  }

  void _updatePositionAnimationState(double delta_time) {
    if (!m_positionState.animationStarted)
      return;

    m_positionState.updateTimer(delta_time);
    float t = m_positionState.getProgress();

    if (t == 1.0)
      m_positionState.animationStarted = false;

    glm::vec3 current_pos =
        glm::mix(m_positionState.start, m_positionState.target, t);

    setPosition(current_pos);
  }

  void _updateAnimation(double delta_time) {
    Animator &animator = m_animator.ensureInitialized();
    animator.updateAnimation(delta_time);
  }
};
