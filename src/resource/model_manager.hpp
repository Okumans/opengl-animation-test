#pragma once

#include "graphics/model.hpp"
#include "utility/enum_map.hpp"
#include "utility/not_initialized.hpp"

#include <memory>

// Define model names here
enum class ModelName { KASANE_TETO };

class ModelManager {
public:
  static SettableNotInitialized<
      EnumMap<ModelName, std::shared_ptr<Model>>, "s_models",
      EnumMapValidator<EnumMap<ModelName, std::shared_ptr<Model>>>>
      s_models;

  static std::shared_ptr<Model>
  loadModel(ModelName name, const char *model_path, bool flip_vertical = false);

  static std::shared_ptr<Model> getModel(ModelName name);

  static bool exists(ModelName name);
};
