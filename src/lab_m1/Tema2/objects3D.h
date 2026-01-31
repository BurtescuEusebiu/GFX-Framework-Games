#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace Objects3D {
Mesh *Create3DGridCube(const std::string &name, glm::vec3 bottomCenter,
                       float length, glm::vec3 color);
Mesh *CreateRiverMesh(const std::string &name, glm::vec3 bottomCenter,
                      float length, float height, glm::vec3 color);
Mesh *CreateMountainMesh(const std::string &name, glm::vec3 bottomCenter,
                         float length, float height, glm::vec3 color);
Mesh *CreatePlainMesh(const std::string &name, glm::vec3 bottomCenter,
                      float length, float width, float height, glm::vec3 color);
Mesh *CreateCentralStationMesh(const std::string &name, glm::vec3 bottomCenter,
                               float length, float height, glm::vec3 colorBase,
                               glm::vec3 colorStack = glm::vec3(0.75f, 0.75f,
                                                                0.75f));
Mesh *
CreateLumberStationMesh(const std::string &name, glm::vec3 bottomCenter,
                        float length, float height, glm::vec3 colorBase,
                        glm::vec3 colorRoof = glm::vec3(0.40f, 0.25f, 0.15f),
                        glm::vec3 colorColumns = glm::vec3(0.40f, 0.25f, 0.20f),
                        glm::vec3 colorLogs = glm::vec3(0.70f, 0.55f, 0.35f));

Mesh *CreateStoneStationMesh(const std::string &name, glm::vec3 bottomCenter,
                             float length, float height,
                             glm::vec3 color1 = glm::vec3(0.45f, 0.45f, 0.45f),
                             glm::vec3 color2 = glm::vec3(0.55f, 0.55f, 0.55f),
                             glm::vec3 color3 = glm::vec3(0.65f, 0.65f, 0.65f));
Mesh *CreateIronStationMesh(const std::string &name, glm::vec3 bottomCenter,
                            float radius, float height,
                            glm::vec3 hillColor = glm::vec3(0.5f, 0.5f, 0.5f),
                            glm::vec3 entranceColor = glm::vec3(0.1f, 0.1f,
                                                                0.1f));
Mesh *CreateStraightRailMesh(const std::string &name, glm::vec3 bottomCenter,
                             float railLength, float railHeight,
                             float railWidth, glm::vec3 color);
Mesh *CreateBridgeMesh(const std::string &name, glm::vec3 bottomCenter,
                       float length, float width, float height,
                       glm::vec3 colorBridge, glm::vec3 colorRails);
Mesh *CreateCornerRailMesh(const std::string &name, glm::vec3 bottomCenter,
                           float length, float railWidth, float railHeight,
                           glm::vec3 color);

Mesh *CreateMountainTunnelMesh(const std::string &name, glm::vec3 bottomCenter,
                               float baseLength, float height, glm::vec3 color);

Mesh *CreateTrainMesh(const std::string &name, glm::vec3 color,
                      float size = 1.0f);

Mesh *CreateWagonMesh(const std::string &name, glm::vec3 color, float size = 1.0f);
} // namespace Objects3D

