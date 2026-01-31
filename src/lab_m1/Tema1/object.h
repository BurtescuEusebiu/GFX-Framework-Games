#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace object {

// Create square with given bottom left corner, length and color
Mesh *CreateSquare(const std::string &name, glm::vec3 leftBottomCorner,
                   float length, glm::vec3 color, bool fill = false);
Mesh *CreateRectangle(const std::string &name, glm::vec3 leftBottomCorner,
                      float length, float width, glm::vec3 color,
                      bool fill = false);
Mesh *CreateStartButton(const std::string &name, glm::vec3 leftBottomCorner,
                        float length, glm::vec3 color);
Mesh *CreateLine(const std::string &name, glm::vec3 start, glm::vec3 end,
                 glm::vec3 color);
Mesh *CreateEngine(const std::string &name, glm::vec3 leftBottomCorner,
                   float length);
Mesh *CreateCannon(const std::string &name, glm::vec3 leftBottomCorner,
                   float length);
Mesh *CreateBumper(const std::string &name, glm::vec3 leftBottomCorner,
                   float length);
Mesh *CreateEnemy(const std::string &name, glm::vec3 leftBottomCorner,
                  float length, float width, int rng);
Mesh *CreateCircle(const std::string &name, glm::vec3 center, float radius,
                   glm::vec3 color, int numSegments = 50);
Mesh *CreateHeart(const std::string &name, glm::vec3 center, float size,
                  glm::vec3 color, int numSegments = 50);

} // namespace object
