#include "object.h"

#include <vector>

#include "core/engine.h"
#include "utils/gl_utils.h"

Mesh *object::CreateSquare(const std::string &name, glm::vec3 leftBottomCorner,
                           float length, glm::vec3 color, bool fill) {
  glm::vec3 corner = leftBottomCorner;

  std::vector<VertexFormat> vertices = {
      VertexFormat(corner, color),
      VertexFormat(corner + glm::vec3(length, 0, 0), color),
      VertexFormat(corner + glm::vec3(length, length, 0), color),
      VertexFormat(corner + glm::vec3(0, length, 0), color)};

  Mesh *square = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 2, 3};

  if (!fill) {
    square->SetDrawMode(GL_LINE_LOOP);
  } else {
    // Draw 2 triangles. Add the remaining 2 indices
    indices.push_back(0);
    indices.push_back(2);
  }

  square->InitFromData(vertices, indices);
  return square;
}

Mesh *object::CreateRectangle(const std::string &name,
                              glm::vec3 leftBottomCorner, float length,
                              float width, glm::vec3 color, bool fill) {
  glm::vec3 corner = leftBottomCorner;

  std::vector<VertexFormat> vertices = {
      VertexFormat(corner, color),
      VertexFormat(corner + glm::vec3(length, 0, 0), color),
      VertexFormat(corner + glm::vec3(length, width, 0), color),
      VertexFormat(corner + glm::vec3(0, width, 0), color)};

  Mesh *rectangle = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 2, 3};

  if (!fill) {
    rectangle->SetDrawMode(GL_LINE_LOOP);
  } else {
    // Draw 2 triangles. Add the remaining 2 indices
    indices.push_back(0);
    indices.push_back(2);
  }

  rectangle->InitFromData(vertices, indices);
  return rectangle;
}

Mesh *object::CreateStartButton(const std::string &name,
                                glm::vec3 leftBottomCorner, float length,
                                glm::vec3 color) {
  glm::vec3 corner = leftBottomCorner;

  std::vector<VertexFormat> vertices = {
      VertexFormat(corner, color),
      VertexFormat(corner + glm::vec3(length, 0, 0), color),
      VertexFormat(corner + glm::vec3(length, length, 0), color),
      VertexFormat(corner + glm::vec3(0, length, 0), color)};

  Mesh *rectangle = new Mesh(name);
  std::vector<unsigned int> indices = {0, 2, 3, 3, 1, 0};

  rectangle->InitFromData(vertices, indices);
  return rectangle;
}

Mesh *object::CreateLine(const std::string &name, glm::vec3 start,
                         glm::vec3 end, glm::vec3 color) {

  std::vector<VertexFormat> vertices = {VertexFormat(start, color),
                                        VertexFormat(end, color)};

  Mesh *line = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1};

  line->SetDrawMode(GL_LINES);
  line->InitFromData(vertices, indices);
  return line;
}

Mesh *object::CreateEngine(const std::string &name, glm::vec3 leftBottomCorner,
                           float length) {
  glm::vec3 corner = leftBottomCorner;

  glm::vec3 color = glm::vec3(1, 0.75f, 0);
  glm::vec3 fireColor = glm::vec3(1, 0.50f, 0);
  std::vector<VertexFormat> vertices = {
      // engine block
      VertexFormat(corner, color),                                // 0
      VertexFormat(corner + glm::vec3(length, 0, 0), color),      // 1
      VertexFormat(corner + glm::vec3(length, length, 0), color), // 2
      VertexFormat(corner + glm::vec3(0, length, 0), color),      // 3

      // engine fire
      VertexFormat(corner + glm::vec3(5, 0, 0), fireColor),   // 4 a
      VertexFormat(corner + glm::vec3(5, -50, 0), fireColor), // 5 b
      VertexFormat(corner + glm::vec3(25, 0, 0), fireColor),  // 6 c

      VertexFormat(corner + glm::vec3(55, 0, 0), fireColor),   // 7 d
      VertexFormat(corner + glm::vec3(55, -50, 0), fireColor), // 8 e
      VertexFormat(corner + glm::vec3(35, 0, 0), fireColor),   // 9 f

      VertexFormat(corner + glm::vec3(15, -50, 0), fireColor), // 10 g
      VertexFormat(corner + glm::vec3(45, -50, 0), fireColor), // 11 h
      VertexFormat(corner + glm::vec3(30, -50, 0), fireColor)  // 12 i
  };

  Mesh *square = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 2,  3, 0,  2, 4, 5, 6,  7, 8,
                                       9, 6, 10, 7, 11, 9, 4, 4, 12, 7};

  square->InitFromData(vertices, indices);
  return square;
}

Mesh *object::CreateCannon(const std::string &name, glm::vec3 leftBottomCorner,
                           float length) {
  glm::vec3 corner = leftBottomCorner;
  glm::vec3 semiCircleCenter =
      corner + glm::vec3(length / 2, length / 2, 0.01f);
  glm::vec3 color = glm::vec3(0.50f, 0.50f, 0.50f);
  glm::vec3 barrelColor = glm::vec3(0.25f, 0.25f, 0.25f);
  std::vector<VertexFormat> vertices = {
      // engine block
      VertexFormat(corner, color),                                    // 0
      VertexFormat(corner + glm::vec3(length, 0, 0), color),          // 1
      VertexFormat(corner + glm::vec3(length, length / 2, 0), color), // 2
      VertexFormat(corner + glm::vec3(0, length / 2, 0), color),      // 3

      VertexFormat(corner + glm::vec3(4, length / 2, 0), barrelColor), // 4
      VertexFormat(corner + glm::vec3(length - 5, length / 2, 0),
                   barrelColor),                                           // 5
      VertexFormat(corner + glm::vec3(4, length * 5 / 2, 0), barrelColor), // 6
      VertexFormat(corner + glm::vec3(length - 5, length * 5 / 2, 0),
                   barrelColor) // 7
  };

  int nrSegments = 10;

  for (int i = 0; i <= nrSegments; i++) {
    float angle = M_PI * i / nrSegments;
    float x = semiCircleCenter.x + length / 2 * cos(angle);
    float y = semiCircleCenter.y + length / 2 * sin(angle);
    vertices.push_back(VertexFormat(glm::vec3(x, y, 0.01f), color));
  }

  Mesh *square = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 2, 3, 0, 2, 4, 5, 6, 6, 7, 5};

  unsigned int centerIndex = vertices.size();
  vertices.push_back(VertexFormat(semiCircleCenter, color));

  for (int i = 0; i < nrSegments; i++) {
    indices.push_back(centerIndex);
    indices.push_back(8 + i);
    indices.push_back(8 + i + 1);
  }

  square->InitFromData(vertices, indices);
  return square;
}

Mesh *object::CreateBumper(const std::string &name, glm::vec3 leftBottomCorner,
                           float length) {
  glm::vec3 corner = leftBottomCorner;
  glm::vec3 semiCircleCenter = corner + glm::vec3(length / 2, length, 0.01f);
  glm::vec3 color = glm::vec3(0.90f, 0.83f, 0.68f);
  std::vector<VertexFormat> vertices = {
      // engine block
      VertexFormat(corner, color),                                // 0
      VertexFormat(corner + glm::vec3(length, 0, 0), color),      // 1
      VertexFormat(corner + glm::vec3(length, length, 0), color), // 2
      VertexFormat(corner + glm::vec3(0, length, 0), color),      // 3
  };

  int nrSegments = 10;

  for (int i = 0; i <= nrSegments; i++) {
    float angle = M_PI * i / nrSegments;
    float x = semiCircleCenter.x + length * 3 / 2 * cos(angle);
    float y = semiCircleCenter.y + length * sin(angle);
    vertices.push_back(VertexFormat(glm::vec3(x, y, 0.01f), color));
  }

  Mesh *square = new Mesh(name);
  std::vector<unsigned int> indices = {0, 1, 2, 3, 0, 2};

  unsigned int centerIndex = vertices.size();
  vertices.push_back(VertexFormat(semiCircleCenter, color));

  for (int i = 0; i < nrSegments; i++) {
    indices.push_back(centerIndex);
    indices.push_back(4 + i);
    indices.push_back(4 + i + 1);
  }

  square->InitFromData(vertices, indices);
  return square;
}

Mesh *object::CreateCircle(const std::string &name, glm::vec3 center,
                           float radius, glm::vec3 color, int numSegments) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  vertices.push_back(VertexFormat(center, color));

  for (int i = 0; i <= numSegments; ++i) {
    float angle = 2.0f * glm::pi<float>() * i / numSegments;
    float x = center.x + radius * cos(angle);
    float y = center.y + radius * sin(angle);
    vertices.push_back(VertexFormat(glm::vec3(x, y, 0.0f), color));
  }

  for (int i = 1; i <= numSegments; ++i) {
    indices.push_back(0);
    indices.push_back(i);
    indices.push_back(i + 1);
  }

  Mesh *circle = new Mesh(name);
  circle->SetDrawMode(GL_TRIANGLES);

  circle->InitFromData(vertices, indices);
  return circle;
}

Mesh *object::CreateHeart(const std::string &name, glm::vec3 center, float size,
                          glm::vec3 color, int numSegments) {
  std::vector<VertexFormat> vertices;
  std::vector<unsigned int> indices;

  vertices.push_back(VertexFormat(center, color));

  for (int i = 0; i <= numSegments; ++i) {
    float t = glm::pi<float>() * 2.0f * i / numSegments;

    float x = 16 * pow(sin(t), 3);
    float y = 13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t);

    x /= 18.0f;
    y /= 18.0f;

    x = center.x + x * size;
    y = center.y + y * size;

    vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color));
  }

  for (int i = 1; i <= numSegments; ++i) {
    indices.push_back(0);
    indices.push_back(i);
    indices.push_back(i + 1);
  }

  Mesh *heart = new Mesh(name);
  heart->SetDrawMode(GL_TRIANGLES);
  heart->InitFromData(vertices, indices);

  return heart;
}
