#pragma once

#include <assimp/scene.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <cassert>
#include <string>
#include <vector>

struct KeyPosition {
  glm::vec3 position;
  float timeStamp;
};

struct KeyRotation {
  glm::quat orientation;
  float timeStamp;
};

struct KeyScale {
  glm::vec3 scale;
  float timeStamp;
};

class Bone {
private:
  std::vector<KeyPosition> m_positions;
  std::vector<KeyRotation> m_rotations;
  std::vector<KeyScale> m_scales;

  int m_numPositions;
  int m_numRotations;
  int m_numScalings;

  glm::mat4 m_localTransform;
  std::string m_name;
  int m_id;

public:
  Bone(const std::string &name, int id, const aiNodeAnim *channel);

  void update(float animation_time);

  glm::mat4 getLocalTransform() const { return m_localTransform; }
  std::string getBoneName() const { return m_name; }
  int getBoneID() const { return m_id; }

  int getPositionIndex(float animation_time) const;
  int getRotationIndex(float animation_time) const;
  int getScaleIndex(float animation_time) const;

private:
  float _getScaleFactor(float last_timestamp, float next_timestamp, float animation_time) const;
  glm::mat4 _interpolatePosition(float animation_time) const;
  glm::mat4 _interpolateRotation(float animation_time) const;
  glm::mat4 _interpolateScaling(float animation_time) const;
};
