#pragma once

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float2.hpp"
#include <vector>

namespace m1 {

enum class ComponentType { None, Block, Engine, Cannon, Bumper, Occupied };

struct Cell {
  ComponentType type = ComponentType::None;
  int nrOc = 0;
  glm::vec2 bottomLeft;
  glm::vec2 topRight;
};

class Grid {
public:
  int rows, columns;
  int componentsNr;
  bool ok;
  std::vector<std::vector<Cell>> grid;

  Grid(int rows = 5, int cols = 5, int componentLeft = 10, bool ok = true)
      : rows(rows), columns(cols), componentsNr(componentLeft), ok(ok),
        grid(rows, std::vector<Cell>(cols)) {}

  bool IsValidCell(int row, int col) {
    return row >= 0 && col >= 0 && row < rows && col < columns &&
           grid[row][col].type == ComponentType::None;
  }

  bool isValidDFS(int row, int col) {
    return row >= 0 && col >= 0 && row < rows && col < columns &&
           grid[row][col].type != ComponentType::None &&
           grid[row][col].type != ComponentType::Occupied;
  }

  bool isOverlapped(int row, int col) { return grid[row][col].nrOc > 1; }

  void PlaceComponent(int row, int col, ComponentType type) {
    switch (type) {
    case ComponentType::Block:
      if (IsValidCell(row, col)) {
        grid[row][col].type = type;
        componentsNr--;
      }
      break;
    case ComponentType::Cannon:
      if (IsValidCell(row, col) && IsValidCell(row, col + 1) &&
          IsValidCell(row, col + 2)) {
        grid[row][col].type = type;
        componentsNr--;

        for (int i = col + 1; i <= columns; ++i) {
          if (!IsValidCell(row, i) &&
              GetComponent(row, i) != ComponentType::Occupied) {
            RemoveComponent(row, i);
          }

          grid[row][i].type = ComponentType::Occupied;
          grid[row][i].nrOc++;
        }
      }
      break;

    case ComponentType::Engine:
      if (IsValidCell(row, col) && IsValidCell(row, col - 1)) {
        grid[row][col].type = type;
        componentsNr--;

        for (int i = col - 1; i >= 0; --i) {

          if (!IsValidCell(row, i) &&
              GetComponent(row, i) != ComponentType::Occupied) {
            RemoveComponent(row, i);
          }
          grid[row][i].type = ComponentType::Occupied;
          grid[row][i].nrOc++;
        }
      }
      break;

    case ComponentType::Bumper:
      if (IsValidCell(row, col) && IsValidCell(row, col + 1) &&
          IsValidCell(row + 1, col + 1) && IsValidCell(row + 1, col + 1)) {
        grid[row][col].type = type;
        componentsNr--;

        for (int i = col + 1; i <= columns; ++i) {
          if (!IsValidCell(row - 1, i) &&
              GetComponent(row - 1, i) != ComponentType::Occupied) {
            RemoveComponent(row - 1, i);
          }

          if (!IsValidCell(row, i) &&
              GetComponent(row, i) != ComponentType::Occupied) {
            RemoveComponent(row, i);
          }

          if (!IsValidCell(row + 1, i) &&
              GetComponent(row + 1, i) != ComponentType::Occupied) {
            RemoveComponent(row + 1, i);
          }

          grid[row - 1][i].type = ComponentType::Occupied;
          grid[row][i].type = ComponentType::Occupied;
          grid[row + 1][i].type = ComponentType::Occupied;
          grid[row - 1][i].nrOc++;
          grid[row][i].nrOc++;
          grid[row + 1][i].nrOc++;
        }
      }
      break;
    default:
      break;
    }
  }

  void RemoveComponent(int row, int col) {
    switch (GetComponent(row, col)) {
    case ComponentType::Block:
      grid[row][col].type = ComponentType::None;
      componentsNr++;
      break;
    case ComponentType::Cannon:

      grid[row][col].type = ComponentType::None;
      componentsNr++;
      for (int i = col + 1; i <= columns; ++i) {
        grid[row][i].nrOc--;
        if (grid[row][col].nrOc == 0)
          grid[row][i].type = ComponentType::None;
      }
      break;

    case ComponentType::Engine:
      grid[row][col].type = ComponentType::None;
      componentsNr++;
      for (int i = col - 1; i >= 0; --i) {
        grid[row][i].nrOc--;
        if (grid[row][i].nrOc == 0)
          grid[row][i].type = ComponentType::None;
      }

      break;

    case ComponentType::Bumper:
      grid[row][col].type = ComponentType::None;
      for (int i = col + 1; i <= columns; ++i) {
        grid[row - 1][i].nrOc--;
        grid[row + 1][i].nrOc--;
        grid[row][i].nrOc--;
        if (grid[row - 1][i].nrOc == 0)
          grid[row - 1][i].type = ComponentType::None;
        if (grid[row][i].nrOc == 0)
          grid[row][i].type = ComponentType::None;
        if (grid[row + 1][i].nrOc == 0)
          grid[row + 1][i].type = ComponentType::None;
      }
      componentsNr++;

      break;
    default:
      break;
    }
  }

  ComponentType GetComponent(int row, int col) const {
    return grid[row][col].type;
  }

  void dfs(std::vector<std::vector<bool>> &viz, int i, int j) {
    viz[i][j] = true;

    if (isValidDFS(i - 1, j) && !viz[i - 1][j])
      dfs(viz, i - 1, j);
    if (isValidDFS(i + 1, j) && !viz[i + 1][j])
      dfs(viz, i + 1, j);
    if (isValidDFS(i, j - 1) && !viz[i][j - 1])
      dfs(viz, i, j - 1);
    if (isValidDFS(i, j + 1) && !viz[i][j + 1])
      dfs(viz, i, j + 1);
  }

  bool checkConnected() {
    std::vector<std::vector<bool>> viz(rows, std::vector<bool>(columns));
    bool first = false;
    for (int i = 0; i < rows && first == false; ++i)
      for (int j = 0; j < columns && first == false; ++j) {
        if (isValidDFS(i, j) && !viz[i][j]) {
          dfs(viz, i, j);
          first = true;
        }
      }

    if (!first)
      return false;

    for (int i = 0; i < rows; ++i)
      for (int j = 0; j < columns; ++j)
        if (isValidDFS(i, j) && !viz[i][j])
          return false;
    return true;
  }
};

class Tema1 : public gfxc::SimpleScene {
public:
  Tema1();
  ~Tema1();

  void Init() override;

private:
  void FrameStart() override;
  void Update(float deltaTimeSeconds) override;
  void UpdateEditor(float deltaTimeSeconds);
  void UpdateGame(float deltaTimeSeconds);
  void UpdateEnd(float deltaTimeSeconds);
  void FrameEnd() override;

  void OnInputUpdate(float deltaTime, int mods) override;
  void OnKeyPress(int key, int mods) override;
  void OnKeyRelease(int key, int mods) override;
  void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
  void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
  void OnWindowResize(int width, int height) override;

protected:
  float cx, cy;
  float fx, fy;
  glm::mat3 modelMatrix;
  float translateX, translateY;
  float scaleX, scaleY;
  float angularStep;
  // TODO(student): If you need any other class variables, define them here.

  int shipComponents;
  float editorComponentsSize;
  int componentLeftLength;

  glm::ivec2 resolution;

  glm::vec2 gridSize;

  int componentsX;
  int componentsY;

  ComponentType componentSelected;
  std::vector<ComponentType> ComponentTypes;

  glm::vec2 mousePos;

  Grid grid;

  // game related stuff

  bool inGame;
  bool lost;
  bool won;

  int livesLeft;
  int bricksLeft;

  int minX, maxX, minY, maxY;

  gfxc::TextRenderer *text;

  std::vector<std::vector<int>> bricks;
  std::vector<std::vector<float>> bricksTimer;
  float brickLen;
  int brickWidth;

  glm::vec2 speed;
  float angle;

  glm::vec2 ballUpdate;
  glm::vec2 shipUpdate;

  glm::vec2 shipLocation;
  glm::vec2 ballLocation;

  glm::vec2 prevBallLocation;

  bool started;
  bool start;

  struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    float life;
  };
  std::vector<Particle> particles;

  int score;

  float shakeTime;
  float shakeDuration;
  float shakeStrength;
  glm::vec2 shakeOffset;
};
} // namespace m1
