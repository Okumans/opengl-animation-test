#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct BoneInfo {
  uint32_t id;
  glm::mat4 offset;
};
