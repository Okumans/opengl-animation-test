#pragma once

#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "graphics/animation.hpp"
#include "graphics/animation_state.hpp"
#include "graphics/animator.hpp"
#include "scene/game_object.hpp"
#include "utility/not_initialized.hpp"
#include "utility/utility.hpp"
#include <cassert>
#include <memory>

template <typename AnimationTypes>
  requires std::is_enum_v<AnimationTypes> && requires {
    AnimationTypes::IDLE;
    AnimationTypes::WALKING;
  }
class HumaniodEntity : public GameObject {
protected:
  SettableNotInitialized<
      EnumMap<AnimationTypes, std::shared_ptr<Animation>>, "m_animations",
      EnumMapValidator<EnumMap<AnimationTypes, std::shared_ptr<Animation>>>>
      m_animations;
  NotInitialized<Animator, "m_animator"> m_animator;
  AnimationTypes m_playingAnimation = AnimationTypes::IDLE;

  AnimationState<float> m_rotateState;
  AnimationState<glm::vec3> m_positionState;

public:
  HumaniodEntity(std::shared_ptr<Model> model, glm::vec3 pos = glm::vec3(0.0f),
                 glm::vec3 scale = glm::vec3(1.0f),
                 glm::vec3 rotation = glm::vec3(0.0f))
      : GameObject(model, pos, scale, rotation) {}

  virtual void setup() = 0;
  virtual void moveToWithAnimation(glm::vec3 target) {
    moveWithAnimation(target - m_position);
  }
  virtual void moveTo(glm::vec3 target) { move(target - m_position); }
  virtual void moveWithAnimation(glm::vec3 vec) {
    if (glm::length(vec) < 0.001f)
      return;

    glm::vec3 new_target_pos = m_position + vec;
    float target_yaw = glm::degrees(std::atan2(vec.x, vec.z));

    _setAnimation(AnimationTypes::WALKING);

    m_positionState.startAnimation(m_position, new_target_pos);
    m_rotateState.startAnimation(m_rotation.y, target_yaw);

    m_positionState.timer = 0;
    m_rotateState.timer = 0;
  }

  virtual void move(glm::vec3 vec) {
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
      _setAnimation(AnimationTypes::IDLE);
    }

    _updateAnimation(delta_time);
  }

  virtual void draw(const RenderContext &ctx) override {
    if (!m_model)
      return;

    ctx.shader.setBool("u_HasAnimation", true);

    _updateTransform();
    ctx.shader.setMat4("u_Model", m_modelMatrix);
    m_animator.ensureInitialized().apply(ctx.shader);

    m_model->draw(ctx);

    ctx.shader.setBool("u_HasAnimation", false);
  }

protected:
  virtual void _setAnimation(AnimationTypes animation) {
    if (animation != m_playingAnimation) {
      m_playingAnimation = animation;
      float blend_duration =
          (m_playingAnimation == AnimationTypes::WALKING) ? 0.12f : 0.18f;
      m_animator.ensureInitialized().playAnimation(
          m_animations.ensureInitialized()
              .get_checked(m_playingAnimation)
              .get(),
          blend_duration);
    }
  }

  virtual void _updateRotateAnimationState(double delta_time) {
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

  virtual void _updatePositionAnimationState(double delta_time) {
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

  virtual void _updateAnimation(double delta_time) {
    Animator &animator = m_animator.ensureInitialized();
    animator.updateAnimation(delta_time);
  }

  inline virtual void _setupAnimationDuration() {
    m_positionState.duration.init(0.3f);
    m_rotateState.duration.init(0.2f);
  }
};
