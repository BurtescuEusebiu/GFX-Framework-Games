#include "lab_m1/Tema2/objects3D.h"
#include <vector>

#include "core/engine.h"
#include "core/gpu/mesh.h"
#include "core/gpu/vertex_format.h"
#include "utils/gl_utils.h"

void AddBox(std::vector<VertexFormat> &vertices,
            std::vector<unsigned int> &indices, const glm::vec3 &center,
            float width, float height, float depth, const glm::vec3 &color) {
  unsigned int startIndex = vertices.size();

  float lengthX = width * 0.5f;
  float lengthY = height * 0.5f;
  float lengthZ = depth * 0.5f;

  vertices.emplace_back(center + glm::vec3(-lengthX, -lengthY, -lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(lengthX, -lengthY, -lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(lengthX, lengthY, -lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(-lengthX, lengthY, -lengthZ),
                        color);

  vertices.emplace_back(center + glm::vec3(-lengthX, -lengthY, lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(lengthX, -lengthY, lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(lengthX, lengthY, lengthZ),
                        color);
  vertices.emplace_back(center + glm::vec3(-lengthX, lengthY, lengthZ),
                        color);

  indices.insert(indices.end(),
                 {// Bottom
                  startIndex + 0, startIndex + 1, startIndex + 5,
                  startIndex + 0, startIndex + 5, startIndex + 4,

                  // Top
                  startIndex + 3, startIndex + 6, startIndex + 2,
                  startIndex + 3, startIndex + 7, startIndex + 6,

                  // Front
                  startIndex + 4, startIndex + 5, startIndex + 6,
                  startIndex + 4, startIndex + 6, startIndex + 7,

                  // Back
                  startIndex + 0, startIndex + 2, startIndex + 1,
                  startIndex + 0, startIndex + 3, startIndex + 2,

                  // Left
                  startIndex + 0, startIndex + 7, startIndex + 3,
                  startIndex + 0, startIndex + 4, startIndex + 7,

                  // Right
                  startIndex + 1, startIndex + 2, startIndex + 6,
                  startIndex + 1, startIndex + 6, startIndex + 5});
}
void AddFilledCylinder(std::vector<VertexFormat> &vertices,
                       std::vector<unsigned int> &indices, glm::vec3 center,
                       float radius, float length, int segments,
                       glm::vec3 color,
                       glm::vec3 direction = glm::vec3(1, 0, 0)) {
  unsigned int startIndex = vertices.size();
  float halfLength = length * 0.5f;

  glm::vec3 dirNorm = glm::normalize(direction);
  glm::vec3 defaultAxis(1, 0, 0);
  glm::mat3 rotMat(1.0f);

  if (glm::length(dirNorm - defaultAxis) > 0.001f) {
    glm::vec3 axis = glm::cross(defaultAxis, dirNorm);
    float angle = acos(glm::dot(defaultAxis, dirNorm));
    if (glm::length(axis) > 0.001f) {
      axis = glm::normalize(axis);
      rotMat = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
    }
  }

  for (int i = 0; i <= segments; i++) {
    float angle = 2.0f * M_PI * i / segments;
    float y = radius * cos(angle);
    float z = radius * sin(angle);

    glm::vec3 posA = rotMat * glm::vec3(-halfLength, y, z) + center;
    glm::vec3 posB = rotMat * glm::vec3(halfLength, y, z) + center;

    vertices.emplace_back(posA, color);
    vertices.emplace_back(posB, color);
  }

  for (int i = 0; i < segments; i++) {
    unsigned int a = startIndex + i * 2;
    unsigned int b = a + 1;
    unsigned int c = a + 2;
    unsigned int d = a + 3;

    indices.insert(indices.end(), {a, b, c});
    indices.insert(indices.end(), {b, d, c});
  }

  unsigned int frontCenter = vertices.size();
  vertices.emplace_back(center + rotMat * glm::vec3(-halfLength, 0, 0), color);

  unsigned int backCenter = vertices.size();
  vertices.emplace_back(center + rotMat * glm::vec3(halfLength, 0, 0), color);

  for (int i = 0; i < segments; i++) {
    unsigned int A = startIndex + i * 2;
    unsigned int B = startIndex + (i + 1) * 2;

    indices.insert(indices.end(), {frontCenter, B, A});

    indices.insert(indices.end(), {backCenter, A + 1, B + 1});
  }
}

Mesh *Objects3D::Create3DGridCube(const std::string &name,
                                  glm::vec3 bottomCenter, float length,
                                  glm::vec3 color) {
  glm::vec3 corner = bottomCenter - glm::vec3(length / 2, 0, length / 2);

  std::vector<VertexFormat> vertices = {
      // Bottom face
      VertexFormat(corner, color),
      VertexFormat(corner + glm::vec3(length, 0, 0), color),
      VertexFormat(corner + glm::vec3(length, 0, length), color),
      VertexFormat(corner + glm::vec3(0, 0, length), color),
      // Top face
      VertexFormat(corner + glm::vec3(0, length, 0), color),
      VertexFormat(corner + glm::vec3(length, length, 0), color),
      VertexFormat(corner + glm::vec3(length, length, length), color),
      VertexFormat(corner + glm::vec3(0, length, length), color),
  };

  Mesh *mesh = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                       6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

  mesh->SetDrawMode(GL_LINES);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateRiverMesh(const std::string &name,
                                 glm::vec3 bottomCenter, float length,
                                 float height, glm::vec3 color) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  glm::vec3 center = bottomCenter + glm::vec3(0, height / 2.0f - 0.1f, 0);

  AddBox(vertices, indices, center, length, height, length, color);

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateMountainMesh(const std::string &name,
                                    glm::vec3 bottomCenter, float baseLength,
                                    float height, glm::vec3 color) {
  glm::vec3 corner =
      bottomCenter - glm::vec3(baseLength / 2, 1.0f, baseLength / 2);

  glm::vec3 snowColor = glm::vec3(1.0f, 1.0f, 1.0f);

  std::vector<VertexFormat> vertices = {
      // Base
      VertexFormat(corner, color),
      VertexFormat(corner + glm::vec3(baseLength, 0, 0), color),
      VertexFormat(corner + glm::vec3(baseLength, 0, baseLength), color),
      VertexFormat(corner + glm::vec3(0, 0, baseLength), color),

      // Snow part
      VertexFormat(corner +
                       glm::vec3(baseLength / 4, height * 0.6f, baseLength / 4),
                   snowColor),
      VertexFormat(
          corner + glm::vec3(3 * baseLength / 4, height * 0.6f, baseLength / 4),
          snowColor),
      VertexFormat(corner + glm::vec3(3 * baseLength / 4, height * 0.6f,
                                      3 * baseLength / 4),
                   snowColor),
      VertexFormat(
          corner + glm::vec3(baseLength / 4, height * 0.6f, 3 * baseLength / 4),
          snowColor),

      // Apex
      VertexFormat(corner + glm::vec3(baseLength / 2, height, baseLength / 2),
                   snowColor)};

  std::vector<unsigned int> indices = {// Base
                                       0, 1, 2, 0, 2, 3,

                                       // Sides
                                       0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2, 2, 6,
                                       7, 2, 7, 3, 3, 7, 4, 3, 4, 0,

                                       // Snow sides
                                       4, 8, 5, 5, 8, 6, 6, 8, 7, 7, 8, 4};
  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreatePlainMesh(const std::string &name,
                                 glm::vec3 bottomCenter, float length,
                                 float width, float height, glm::vec3 color) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  glm::vec3 center = bottomCenter - glm::vec3(0, height / 2.0f, 0);

  AddBox(vertices, indices, center, length, height, width, color);

  Mesh *mesh = new Mesh(name);
  mesh->SetDrawMode(GL_TRIANGLES);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateCentralStationMesh(const std::string &name,
                                          glm::vec3 bottomCenter, float length,
                                          float height, glm::vec3 colorBase,
                                          glm::vec3 colorStack) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  AddBox(vertices, indices, bottomCenter + glm::vec3(0, height / 2.0f, 0),
         length, height, length, colorBase);

  int nr = 3;
  float hornRadius = length / 12.0f;
  float hornHeight = height * 0.4f;
  int segments = 24;

  for (int i = 0; i < nr; ++i) {
    glm::vec3 stackCenter =
        bottomCenter + glm::vec3(-length / 3 + i * (length / 3),
                                 height + hornHeight / 2.0f, 0);

    AddFilledCylinder(vertices, indices, stackCenter, hornRadius, hornHeight,
                      segments, colorStack, glm::vec3(0, 1, 0));
  }

  Mesh *mesh = new Mesh(name);
  mesh->SetDrawMode(GL_TRIANGLES);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateLumberStationMesh(const std::string &name,
                                         glm::vec3 bottomCenter, float length,
                                         float height, glm::vec3 colorBase,
                                         glm::vec3 colorRoof,
                                         glm::vec3 colorColumns,
                                         glm::vec3 colorLogs) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  // Base
  AddBox(vertices, indices, bottomCenter + glm::vec3(0, height / 2.0f, 0),
         length, height, length, colorBase);

  // Roof
  float roofHeight = height * 0.15f;
  float roofExtension = length * 0.3f;
  glm::vec3 roofCenter =
      bottomCenter + glm::vec3(0, height + roofHeight / 2.0f, 0);
  AddBox(vertices, indices, roofCenter, length + 2 * roofExtension, roofHeight,
         length + 2 * roofExtension, colorRoof);

  // Columns
  float colWidth = length * 0.08f;
  glm::vec3 col1Center =
      bottomCenter + glm::vec3(-length * 0.3f + colWidth / 2.0f, height / 2.0f,
                               length * 0.6f - colWidth / 2.0f);
  glm::vec3 col2Center = col1Center + glm::vec3(length * 0.6f, 0, 0);

  AddBox(vertices, indices, col1Center, colWidth, height, colWidth,
         colorColumns);
  AddBox(vertices, indices, col2Center, colWidth, height, colWidth,
         colorColumns);

  // Logs
  float logHeight = height * 0.15f;
  glm::vec3 logCenter =
      bottomCenter +
      glm::vec3(0, logHeight / 2.0f, length * 0.875f - length * 0.15f / 2.0f);
  AddBox(vertices, indices, logCenter, length * 0.6f, logHeight, length * 0.15f,
         colorLogs);

  Mesh *mesh = new Mesh(name);
  mesh->SetDrawMode(GL_TRIANGLES);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateStoneStationMesh(const std::string &name,
                                        glm::vec3 bottomCenter, float length,
                                        float height, glm::vec3 color1,
                                        glm::vec3 color2, glm::vec3 color3) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  glm::vec3 colors[3] = {color1, color2, color3};

  float minSize = length * 0.15f;
  float maxSize = length * 0.3f;

  glm::vec3 offsets[10] = {{-0.4f, 0, -0.4f},  {0.1f, 0, -0.35f},
                           {0.35f, 0, -0.2f},  {-0.25f, 0, 0.1f},
                           {0.0f, 0, 0.2f},    {0.3f, 0, 0.3f},
                           {-0.35f, 0, 0.35f}, {0.2f, 0, -0.1f},
                           {-0.1f, 0, -0.2f},  {0.25f, 0, 0.0f}};

  float heights[10] = {height * 0.5f, height * 0.45f, height * 0.38f,
                       height * 0.4f, height * 0.2f,  height * 0.35f,
                       height * 0.5f, height * 0.3f,  height * 0.2f,
                       height * 0.1f};

  for (int i = 0; i < 10; ++i) {
    float rockWidth = minSize + (maxSize - minSize) * (i % 3) / 2.0f;
    float rockDepth = minSize + (maxSize - minSize) * ((i + 1) % 3) / 2.0f;
    float rockHeight = heights[i];

    glm::vec3 boxCenter =
        bottomCenter + offsets[i] * length +
        glm::vec3(rockWidth / 2, rockHeight / 2, rockDepth / 2);

    AddBox(vertices, indices, boxCenter, rockWidth, rockHeight, rockDepth,
           colors[i % 3]);
  }

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateIronStationMesh(const std::string &name,
                                       glm::vec3 bottomCenter, float radius,
                                       float height, glm::vec3 hillColor,
                                       glm::vec3 entranceColor) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  int nrSlices = 16;
  int nrStacks = 8;

  for (int i = 0; i <= nrStacks; ++i) {
    float stackHeight = height * (float)i / nrStacks;
    float stackRadius = radius * (1.0f - (float)i / nrStacks);

    for (int j = 0; j <= nrSlices; ++j) {
      float theta = glm::two_pi<float>() * j / nrSlices;
      float x = stackRadius * cos(theta);
      float z = stackRadius * sin(theta);
      vertices.push_back(
          {bottomCenter + glm::vec3(x, stackHeight, z), hillColor});
    }
  }

  for (int i = 0; i < nrStacks; ++i) {
    for (int j = 0; j < nrSlices; ++j) {
      int first = i * (nrSlices + 1) + j;
      int second = first + nrSlices + 1;

      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);

      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }

  float entranceWidth = radius * 0.4f;
  float entranceHeight = height * 0.2f;
  float entranceDepth = entranceWidth;

  glm::vec3 entranceCenter =
      bottomCenter +
      glm::vec3(0, entranceHeight / 2, -radius + entranceDepth / 1.5f - 0.175f);

  AddBox(vertices, indices, entranceCenter, entranceWidth, entranceHeight,
         entranceDepth, entranceColor);

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateStraightRailMesh(const std::string &name,
                                        glm::vec3 bottomCenter, float length,
                                        float width, float height,
                                        glm::vec3 color) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  float spacing = width * 2.0f;
  glm::vec3 offsets[2] = {glm::vec3(-spacing / 2, 0, 0),
                          glm::vec3(spacing / 2, 0, 0)};

  for (int r = 0; r < 2; ++r) {
    glm::vec3 center = bottomCenter + offsets[r] + glm::vec3(0, height / 2, 0);
    AddBox(vertices, indices, center, width, height, length, color);
  }

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateBridgeMesh(const std::string &name,
                                  glm::vec3 bottomCenter, float length,
                                  float width, float height,
                                  glm::vec3 colorBridge, glm::vec3 colorRails) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  glm::vec3 baseCenter = bottomCenter + glm::vec3(0, height / 2, 0);
  AddBox(vertices, indices, baseCenter, width, height, length, colorBridge);

  float railWidth = 0.25f;
  float railHeight = 0.25f;
  float spacing = railWidth * 2.0f;
  glm::vec3 railOffsets[2] = {glm::vec3(-spacing / 2, height, 0),
                              glm::vec3(spacing / 2, height, 0)};

  for (int r = 0; r < 2; ++r) {
    glm::vec3 railCenter =
        bottomCenter + railOffsets[r] + glm::vec3(0, railHeight / 2, 0);
    AddBox(vertices, indices, railCenter, railWidth, railHeight, length,
           colorRails);
  }

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateCornerRailMesh(const std::string &name,
                                      glm::vec3 bottomCenter, float length,
                                      float width, float height,
                                      glm::vec3 color) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  float spacing = width * 2.0f;
  float shortRailOffset = width * 0.5f;
  float longRailLength = length + 3 * shortRailOffset + shortRailOffset / 2;

  glm::vec3 zOffsets[2] = {glm::vec3(-spacing / 2, 0, -shortRailOffset),
                           glm::vec3(spacing / 2, 0, 3 * shortRailOffset)};

  for (int r = 0; r < 2; ++r) {
    glm::vec3 railCenter = bottomCenter + zOffsets[r];
    float railLength = longRailLength;
    if (r == 0)
      railLength -= 4 * shortRailOffset;

    glm::vec3 center = railCenter + glm::vec3(0, height / 2, -railLength / 2);
    AddBox(vertices, indices, center, width, height, railLength, color);
  }

  glm::vec3 xOffsets[2] = {glm::vec3(-shortRailOffset, 0, -spacing / 2),
                           glm::vec3(3 * shortRailOffset, 0, spacing / 2)};

  for (int r = 0; r < 2; ++r) {
    glm::vec3 railCenter = bottomCenter + xOffsets[r];
    float railLength = longRailLength;
    if (r == 0)
      railLength -= 4 * shortRailOffset;

    glm::vec3 center = railCenter + glm::vec3(-railLength / 2, height / 2, 0);
    AddBox(vertices, indices, center, railLength, height, width, color);
  }

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateMountainTunnelMesh(const std::string &name,
                                          glm::vec3 bottomCenter,
                                          float baseLength, float height,
                                          glm::vec3 color) {
  glm::vec3 corner =
      bottomCenter - glm::vec3(baseLength / 2, 1.0f, baseLength / 2);
  glm::vec3 snowColor = glm::vec3(1.0f, 1.0f, 1.0f);

  float tunnelHeight = baseLength * 0.4f;
  float tunnelWidth = baseLength * 0.3f;
  float tunnelYBottom = 0.0f;
  float tunnelYTop = tunnelYBottom + tunnelHeight;

  float halfTunnelWidth = tunnelWidth / 2.0f;
  float tunnelXMin = bottomCenter.x - halfTunnelWidth;
  float tunnelXMax = bottomCenter.x + halfTunnelWidth;

  float zFront = corner.z;
  float zBack = corner.z + baseLength;

  float ySnowBase = height * 0.6f;
  float yApex = height;

  std::vector<VertexFormat> vertices;

  glm::vec3 v0 = corner;
  glm::vec3 v1 = corner + glm::vec3(baseLength, 0, 0);
  glm::vec3 v2 = corner + glm::vec3(baseLength, 0, baseLength);
  glm::vec3 v3 = corner + glm::vec3(0, 0, baseLength);
  glm::vec3 v4 = corner + glm::vec3(baseLength / 4, ySnowBase, baseLength / 4);
  glm::vec3 v5 =
      corner + glm::vec3(3 * baseLength / 4, ySnowBase, baseLength / 4);
  glm::vec3 v6 =
      corner + glm::vec3(3 * baseLength / 4, ySnowBase, 3 * baseLength / 4);
  glm::vec3 v7 =
      corner + glm::vec3(baseLength / 4, ySnowBase, 3 * baseLength / 4);
  glm::vec3 v8 = corner + glm::vec3(baseLength / 2, yApex, baseLength / 2);

  glm::vec3 f_bl(tunnelXMin, tunnelYBottom, zFront);
  glm::vec3 f_br(tunnelXMax, tunnelYBottom, zFront);
  glm::vec3 f_tl(tunnelXMin, tunnelYTop, zFront);
  glm::vec3 f_tr(tunnelXMax, tunnelYTop, zFront);
  glm::vec3 b_bl(tunnelXMin, tunnelYBottom, zBack);
  glm::vec3 b_br(tunnelXMax, tunnelYBottom, zBack);
  glm::vec3 b_tl(tunnelXMin, tunnelYTop, zBack);
  glm::vec3 b_tr(tunnelXMax, tunnelYTop, zBack);

  vertices.push_back(VertexFormat(v0, color));
  vertices.push_back(VertexFormat(v1, color));
  vertices.push_back(VertexFormat(v2, color));
  vertices.push_back(VertexFormat(v3, color));
  vertices.push_back(VertexFormat(v4, snowColor));
  vertices.push_back(VertexFormat(v5, snowColor));
  vertices.push_back(VertexFormat(v6, snowColor));
  vertices.push_back(VertexFormat(v7, snowColor));
  vertices.push_back(VertexFormat(v8, snowColor));

  vertices.push_back(VertexFormat(f_bl, color));
  vertices.push_back(VertexFormat(f_br, color));
  vertices.push_back(VertexFormat(f_tl, color));
  vertices.push_back(VertexFormat(f_tr, color));
  vertices.push_back(VertexFormat(b_bl, color));
  vertices.push_back(VertexFormat(b_br, color));
  vertices.push_back(VertexFormat(b_tl, color));
  vertices.push_back(VertexFormat(b_tr, color));

  std::vector<unsigned int> indices;

  indices.insert(indices.end(), {0, 9, 11, 0, 11, 4});
  indices.insert(indices.end(), {10, 1, 5, 10, 5, 12});
  indices.insert(indices.end(), {11, 12, 5, 11, 5, 4});

  indices.insert(indices.end(), {3, 13, 15, 3, 15, 7});
  indices.insert(indices.end(), {14, 2, 6, 14, 6, 16});
  indices.insert(indices.end(), {15, 16, 6, 15, 6, 7});

  indices.insert(indices.end(), {0, 3, 7, 0, 7, 4});
  indices.insert(indices.end(), {1, 2, 6, 1, 6, 5});

  indices.insert(indices.end(), {4, 8, 5, 5, 8, 6, 6, 8, 7, 7, 8, 4});

  indices.insert(indices.end(), {9, 10, 14, 9, 14, 13});
  indices.insert(indices.end(), {11, 12, 16, 11, 16, 15});
  indices.insert(indices.end(), {9, 13, 15, 9, 15, 11});
  indices.insert(indices.end(), {10, 14, 16, 10, 16, 12});

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}

Mesh *Objects3D::CreateTrainMesh(const std::string &name, glm::vec3 color,
                                 float size) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;
  glm::vec3 structuralColor = glm::vec3(0.3f, 0.3f, 0.3f);

  float wheelRadius = 0.12f * size;
  float wheelWidth = 0.06f * size;
  float wheelY = 0.12f * size;
  float rowOffset = 0.25f * size;
  float spacing = 0.5f * size;
  int wheelCount = 4;

  // Wheels
  for (int i = 0; i < wheelCount; i++) {
    float z = i * spacing;
    AddFilledCylinder(vertices, indices, glm::vec3(-rowOffset, wheelY, z),
                      wheelRadius, wheelWidth, 24, structuralColor);
    AddFilledCylinder(vertices, indices, glm::vec3(rowOffset, wheelY, z),
                      wheelRadius, wheelWidth, 24, structuralColor);
  }

  float trainLength = (wheelCount - 1) * spacing;
  float platformX = 0.6f * size;
  float platformZ = trainLength + 0.2f * size;
  float platformHeight = 0.1f * size;
  float platformY = wheelY + wheelRadius;

  // Platform
  glm::vec3 platformCenter(0, platformY + platformHeight / 2, trainLength / 2);
  AddBox(vertices, indices, platformCenter, platformX, platformHeight,
         platformZ, structuralColor);

  // Cabin
  float cabinX = 0.4f * size;
  float cabinZ = 0.8f * size;
  float cabinHeight = 0.5f * size;
  glm::vec3 cabinCenter(0, platformY + platformHeight + cabinHeight / 2,
                        trainLength - cabinZ / 2);
  AddBox(vertices, indices, cabinCenter, cabinX, cabinHeight, cabinZ, color);

  // Engine
  float engineRadius = 0.15f * size;
  float engineHeight = 0.8f * size;
  glm::vec3 engineCenter(0, platformY + platformHeight + engineRadius,
                         engineRadius + 0.20f * size);
  AddFilledCylinder(vertices, indices, engineCenter, engineRadius, engineHeight,
                    24, color - glm::vec3(0.1f, 0.1f, 0.1f),
                    glm::vec3(0, 0, 1));

  Mesh *train = new Mesh(name);
  train->InitFromData(vertices, indices);
  return train;
}

Mesh *Objects3D::CreateWagonMesh(const std::string &name, glm::vec3 color,
                                 float size) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;
  glm::vec3 structuralColor = glm::vec3(0.3f, 0.3f, 0.3f);

  float wheelRadius = 0.12f * size;
  float wheelWidth = 0.06f * size;
  float wheelY = 0.12f * size;
  float rowOffset = 0.25f * size;

  float platformZ = 1.6f * size;
  float trainWidth = 0.6f * size;
  float platformHeight = 0.1f * size;
  float platformY = wheelY + wheelRadius;

  float frontZ = wheelRadius;
  float backZ = platformZ - wheelRadius;

  for (int i = 0; i < 2; i++) {
    float z = (i == 0) ? frontZ : backZ;

    AddFilledCylinder(vertices, indices, glm::vec3(-rowOffset, wheelY, z),
                      wheelRadius, wheelWidth, 24, structuralColor);

    AddFilledCylinder(vertices, indices, glm::vec3(rowOffset, wheelY, z),
                      wheelRadius, wheelWidth, 24, structuralColor);
  }

  glm::vec3 platformCenter(0, platformY + platformHeight / 2, platformZ / 2);
  AddBox(vertices, indices, platformCenter, trainWidth, platformHeight,
         platformZ, structuralColor);

  float cabinHeight = 0.5f * size;
  glm::vec3 cabinCenter(0, platformY + platformHeight + cabinHeight / 2,
                        platformZ / 2);
  AddBox(vertices, indices, cabinCenter, trainWidth, cabinHeight, platformZ,
         color);

  Mesh *mesh = new Mesh(name);
  mesh->InitFromData(vertices, indices);
  return mesh;
}
