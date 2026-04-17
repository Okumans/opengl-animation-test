# Vampire Survivor Project - Naming Conventions & Practices

This file serves as a reference for AI agents to maintain consistent coding style, naming conventions, and architectural practices across the C++ codebase.

## Naming Conventions

### 1. Variables & Members
- **Local Variables & Parameters:** `snake_case` (e.g., `bone_transform`, `delta_time`).
- **Member Variables (Private/Protected):** Prefix with `m_` and use `camelCase` (e.g., `m_camera`, `m_testObject`).
- **Static Member Variables:** Prefix with `s_` (e.g., `s_models`, `s_minClipY`).

### 2. Classes, Structs & Enums
- **Classes/Structs:** `PascalCase` (e.g., `GameObject`, `RenderContext`, `CameraController`).
- **Enums:** `PascalCase` for the enum name (e.g., `ModelName`, `GameState`).
- **Enum Values:** `UPPER_SNAKE_CASE` (e.g., `KASANE_TETO`, `GAME_OVER`).

### 3. Methods & Functions
- **Public Methods:** `camelCase` (e.g., `updateAnimation()`, `setBaseColor()`).
- **Private/Protected Methods:** Prefix with an underscore `_` and use `camelCase` (e.g., `_setupMesh()`, `_updateTransform()`).
- **Global Functions/C-Style callbacks:** `camelCase` or descriptive prefixed names.

### 4. Constants & Macros
- **Constants (Const/Constexpr):** `UPPER_SNAKE_CASE` (e.g., `MAX_BONES`, `MAX_BONE_INFLUENCE`).
- **Compiler Macros:** `UPPER_SNAKE_CASE` (e.g., `ASSETS_PATH`).

## Architectural Practices

### Headers vs Sources
- Always use `#pragma once` at the top of headers.
- Include heavy external headers (like `<assimp/...>` or `<glm/gtc/...>`) in the `.cpp` file rather than `.hpp` where possible.
- Use forward declarations (e.g., `#include <glm/fwd.hpp>`) in `.hpp` whenever full definitions are not strictly necessary.

### Memory & Pointers
- Utilize smart pointers over raw `new`/`delete` for object management (e.g., `std::unique_ptr<GameObject>`, `std::shared_ptr<Model>`).
- Pass complex objects to functions by `const &` (e.g., `const RenderContext &ctx`).

### Rendering Conventions
- Use robust standard 3D positional systems using `glm::vec3` (`m_position`, `m_scale`, `m_rotation`).
- Mathematical AABB boundaries should be cached natively (`m_worldAABB`) using the standalone robust `AABB` struct. Lazy-cache transformations heavily where applicable to improve C++ runtime performance over iterations over hundreds of models.

### Assets Loading Workflow
- Follow the loading tasks queue paradigm established inside `App::_setupResources()` to safely load Models, Textures, and Shaders.
- `ModelManager`, `TextureManager`, and `ShaderManager` should be utilized for fetching instantiated assets across the active GL Context block.
