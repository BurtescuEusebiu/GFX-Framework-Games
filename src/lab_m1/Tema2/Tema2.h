#pragma once

#include "components/simple_scene.h"
#include "core/window/input_controller.h"
#include "lab_m1/Tema2/Tema2_camera.h"
#include <deque>
#include <iostream>
#include <utility>

namespace m1 {

enum class TerrainType { Plain, Water, Mountain };

enum class RailType {
  None,
  Straight,
  Corner,
  TRail,
  CrossRail,
  Bridge,
  Tunnel
};

enum class Direction { None, North, East, South, West };

enum class StationType { None, Central, Lumber, Stone, Iron };

struct treeNode {
  int row;
  int col;
  RailType railType;
  Direction railDirection;
  StationType stationType;
  treeNode **children;
  treeNode *parent = nullptr;
  bool hasWaited = false;

  treeNode(int r, int c, RailType rType = RailType::None,
           Direction rDir = Direction::None,
           StationType sType = StationType::None, treeNode *p = nullptr)
      : row(r), col(c), railType(rType), railDirection(rDir),
        stationType(sType), children(nullptr), parent(p) {}
};

struct Train {
  treeNode *rail;
  Direction entryDir;
  float progress;
  float speed;
  glm::vec3 position;
  bool waiting = false;
  int moveDir = 1;
};

struct TrainState {
  treeNode *rail;
  float progress;
  Direction entryDir;
};

struct Order {
  float timeLeft;
  int stuff[5];
  bool completed = false;
  float timers[3] = {5.f, 5.f, 5.f};
};

struct cell3D {
  TerrainType type = TerrainType::Plain;
  RailType railType = RailType::None;
  Direction railDirection = Direction::None;
  StationType stationType = StationType::None;
};

class Grid3D {
public:
  ~Grid3D() { ClearRailTree(); }

  int rows, columns;
  std::vector<std::vector<cell3D>> grid;
  std::map<std::pair<RailType, Direction>, std::vector<Direction>> connections;
  std::pair<int, int> DirectionToOffset(Direction dir) {
    switch (dir) {
    case Direction::North:
      return {-1, 0};
    case Direction::South:
      return {1, 0};
    case Direction::East:
      return {0, 1};
    case Direction::West:
      return {0, -1};
    default:
      return {0, 0};
    }
  }

  void InitializeConnections() {
    connections[{RailType::Straight, Direction::North}] = {Direction::North,
                                                           Direction::South};
    connections[{RailType::Straight, Direction::East}] = {Direction::East,
                                                          Direction::West};
    connections[{RailType::Straight, Direction::South}] = {Direction::North,
                                                           Direction::South};
    connections[{RailType::Straight, Direction::West}] = {Direction::East,
                                                          Direction::West};

    connections[{RailType::Bridge, Direction::North}] = {Direction::North,
                                                         Direction::South};
    connections[{RailType::Bridge, Direction::East}] = {Direction::East,
                                                        Direction::West};
    connections[{RailType::Bridge, Direction::South}] = {Direction::North,
                                                         Direction::South};
    connections[{RailType::Bridge, Direction::West}] = {Direction::East,
                                                        Direction::West};

    connections[{RailType::Tunnel, Direction::North}] = {Direction::North,
                                                         Direction::South};
    connections[{RailType::Tunnel, Direction::East}] = {Direction::East,
                                                        Direction::West};
    connections[{RailType::Tunnel, Direction::South}] = {Direction::North,
                                                         Direction::South};
    connections[{RailType::Tunnel, Direction::West}] = {Direction::East,
                                                        Direction::West};

    connections[{RailType::Corner, Direction::North}] = {Direction::North,
                                                         Direction::East};
    connections[{RailType::Corner, Direction::East}] = {Direction::East,
                                                        Direction::South};
    connections[{RailType::Corner, Direction::South}] = {Direction::South,
                                                         Direction::West};
    connections[{RailType::Corner, Direction::West}] = {Direction::West,
                                                        Direction::North};

    connections[{RailType::TRail, Direction::North}] = {
        Direction::North, Direction::South, Direction::East};
    connections[{RailType::TRail, Direction::East}] = {
        Direction::East, Direction::West, Direction::South};
    connections[{RailType::TRail, Direction::South}] = {
        Direction::South, Direction::West, Direction::North};
    connections[{RailType::TRail, Direction::West}] = {
        Direction::West, Direction::North, Direction::East};

    connections[{RailType::CrossRail, Direction::North}] = {
        Direction::North, Direction::South, Direction::East, Direction::West};
    connections[{RailType::CrossRail, Direction::East}] = {
        Direction::North, Direction::South, Direction::East, Direction::West};
    connections[{RailType::CrossRail, Direction::South}] = {
        Direction::North, Direction::South, Direction::East, Direction::West};
    connections[{RailType::CrossRail, Direction::West}] = {
        Direction::North, Direction::South, Direction::East, Direction::West};
  }

  treeNode *railTreeRoot = nullptr;

  Grid3D(int rows = 10, int cols = 10)
      : rows(rows), columns(cols), grid(rows, std::vector<cell3D>(cols)) {}

  bool IsValidCell(int row, int col) {
    return row >= 0 && col >= 0 && row < rows && col < columns;
  }

  bool IsCellInTree(treeNode *node, int row, int col) {
    if (!node)
      return false;
    if (node->row == row && node->col == col)
      return true;

    if (node->children) {
      for (int i = 0; i < 4; ++i) {
        if (node->children[i] && IsCellInTree(node->children[i], row, col))
          return true;
      }
    }
    return false;
  }

  void PlaceRail(int row, int col, RailType type,
                 Direction direction = Direction::None) {
    if (!IsValidCell(row, col) || grid[row][col].railType != RailType::None)
      return;

    if (grid[row][col].type == TerrainType::Mountain &&
        type != RailType::Straight)
      return;

    if (!railTreeRoot) {
      std::cout << "Cannot place rail: no rail tree exists yet\n";
      return;
    }

    bool found = false;

    auto it = connections.find({type, direction});
    if (it == connections.end())
      return;

    const std::vector<Direction> &dirs = it->second;

    for (Direction dir : dirs) {
      std::pair<int, int> offset = DirectionToOffset(dir);
      int nRow = row + offset.first;
      int nCol = col + offset.second;

      if (!IsValidCell(nRow, nCol))
        continue;

      RailType neighborType = grid[nRow][nCol].railType;
      Direction neighborDir = grid[nRow][nCol].railDirection;
      StationType neighborStation = grid[nRow][nCol].stationType;

      if ((neighborType != RailType::None ||
           neighborStation == StationType::Central) &&
          IsCellInTree(railTreeRoot, nRow, nCol)) {
        Direction reverseDir;
        switch (dir) {
        case Direction::North:
          reverseDir = Direction::South;
          break;
        case Direction::South:
          reverseDir = Direction::North;
          break;
        case Direction::East:
          reverseDir = Direction::West;
          break;
        case Direction::West:
          reverseDir = Direction::East;
          break;
        default:
          reverseDir = Direction::None;
        }

        if (neighborType != RailType::None) {
          auto nIt = connections.find({neighborType, neighborDir});
          if (nIt != connections.end()) {
            const std::vector<Direction> &neighborDirs = nIt->second;
            if (std::find(neighborDirs.begin(), neighborDirs.end(),
                          reverseDir) != neighborDirs.end()) {
              found = true;
              break;
            }
          }
        } else {
          found = true;
          break;
        }
      }
    }

    if (!found) {
      std::cout << "Cannot place rail: no valid connection to the rail tree\n";
      return;
    }

    grid[row][col].railType = type;
    grid[row][col].railDirection = direction;

    std::cout << "Placed rail at (" << row << "," << col << ")\n";

    treeNode *parentNode = nullptr;

    for (Direction dir : dirs) {
      std::pair<int, int> offset = DirectionToOffset(dir);
      int nRow = row + offset.first;
      int nCol = col + offset.second;
      if (IsCellInTree(railTreeRoot, nRow, nCol)) {
        parentNode = FindTreeNode(railTreeRoot, nRow, nCol);
        break;
      }
    }

    if (parentNode) {
      treeNode *newNode = new treeNode(row, col, type, direction,
                                       StationType::None, parentNode);
      if (!parentNode->children)
        parentNode->children = new treeNode *[4]{nullptr};
      for (int i = 0; i < 4; ++i) {
        if (!parentNode->children[i]) {
          parentNode->children[i] = newNode;
          break;
        }
      }
    }
  }

  treeNode *FindTreeNode(treeNode *node, int row, int col) {
    if (!node)
      return nullptr;
    if (node->row == row && node->col == col)
      return node;
    if (node->children) {
      for (int i = 0; i < 4; ++i) {
        if (node->children[i]) {
          treeNode *res = FindTreeNode(node->children[i], row, col);
          if (res)
            return res;
        }
      }
    }
    return nullptr;
  }

  void PlaceStation(int row, int col, StationType type) {
    if (IsValidCell(row, col) && grid[row][col].railType == RailType::None &&
        type != StationType::None) {
      grid[row][col].stationType = type;
    }
  }

  void ClearRailTree() {
    if (railTreeRoot) {
      DeleteTreeNode(railTreeRoot);
      railTreeRoot = nullptr;
    }
  }

  void DeleteTreeNode(treeNode *node) {
    if (!node)
      return;

    if (node->children) {
      for (int i = 0; i < 4; ++i) {
        if (node->children[i]) {
          DeleteTreeNode(node->children[i]);
          node->children[i] = nullptr;
        }
      }
      delete[] node->children;
      node->children = nullptr;
    }
    grid[node->row][node->col].railType = RailType::None;
    grid[node->row][node->col].railDirection = Direction::None;
    delete node;
  }

  void RemoveNodeFromParent(treeNode *node) {
    if (!node || !node->parent)
      return;
    treeNode *parent = node->parent;
    if (parent->children) {
      for (int i = 0; i < 4; ++i) {
        if (parent->children[i] == node) {
          parent->children[i] = nullptr;
          break;
        }
      }
    }
  }

  void RemoveRail(int row, int col) {
    if (!IsValidCell(row, col))
      return;

    if (grid[row][col].railType == RailType::None)
      return;

    treeNode *node = FindTreeNode(railTreeRoot, row, col);
    if (!node)
      return;

    RemoveNodeFromParent(node);

    DeleteTreeNode(node);

    grid[row][col].railType = RailType::None;
    grid[row][col].railDirection = Direction::None;

    std::cout << "Removed rail at (" << row << "," << col << ")\n";
  }

  void AddStationToTree(int row, int col) {
    if (!IsValidCell(row, col))
      return;

    if (grid[row][col].stationType == StationType::None)
      return;

    if (grid[row][col].railType != RailType::None)
      return;

    if (!railTreeRoot) {
      std::cout << "Cannot add station: no rail tree exists\n";
      return;
    }

    if (IsCellInTree(railTreeRoot, row, col))
      return;

    bool found = false;
    treeNode *parentNode = nullptr;

    for (Direction side : {Direction::North, Direction::East, Direction::South,
                           Direction::West}) {

      auto offset = DirectionToOffset(side);
      int nRow = row + offset.first;
      int nCol = col + offset.second;

      if (!IsValidCell(nRow, nCol))
        continue;

      RailType neighborType = grid[nRow][nCol].railType;
      Direction neighborDir = grid[nRow][nCol].railDirection;

      if (neighborType == RailType::None)
        continue;

      if (!IsCellInTree(railTreeRoot, nRow, nCol))
        continue;

      auto it = connections.find({neighborType, neighborDir});
      if (it == connections.end())
        continue;

      const std::vector<Direction> &dirs = it->second;

      Direction toStation;
      switch (side) {
      case Direction::North:
        toStation = Direction::South;
        break;
      case Direction::South:
        toStation = Direction::North;
        break;
      case Direction::East:
        toStation = Direction::West;
        break;
      case Direction::West:
        toStation = Direction::East;
        break;
      default:
        continue;
      }

      if (std::find(dirs.begin(), dirs.end(), toStation) != dirs.end()) {
        found = true;
        parentNode = FindTreeNode(railTreeRoot, nRow, nCol);
        break;
      }
    }

    if (!found || !parentNode) {
      std::cout << "Cannot add station: no valid rail connection\n";
      return;
    }

    treeNode *stationNode =
        new treeNode(row, col, RailType::Straight, Direction::None,
                     grid[row][col].stationType, parentNode);

    if (!parentNode->children)
      parentNode->children = new treeNode *[4]{nullptr};

    for (int i = 0; i < 4; ++i) {
      if (!parentNode->children[i]) {
        parentNode->children[i] = stationNode;
        break;
      }
    }

    std::cout << "Added station to rail tree at (" << row << "," << col
              << ")\n";
  }
};

class Tema2 : public gfxc::SimpleScene {
public:
  Tema2();
  ~Tema2();

  void Init() override;

private:
  void FrameStart() override;
  void Update(float deltaTimeSeconds) override;
  void FrameEnd() override;

  void RenderMesh(Mesh *mesh, Shader *shader,
                  const glm::mat4 &modelMatrix) override;

  void OnInputUpdate(float deltaTime, int mods) override;
  void OnKeyPress(int key, int mods) override;
  void OnKeyRelease(int key, int mods) override;
  void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
  void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
  void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
  void OnWindowResize(int width, int height) override;
  void InitializeTrain();
  void UpdateTrain(float deltaTime);
  void RenderSimpleMesh(Mesh *mesh, Shader *shader,
                        const glm::mat4 &modelMatrix, float timeFactor = 0.f);

protected:
  implemented::Tema2Camera *camera;
  glm::mat4 projectionMatrix;
  bool renderCameraTarget;

public:
  bool editorMode = true;
  int worldSize = 10;
  int cellSizePixels = 5;
  Grid3D worldGrid;
  int riverDirection = 0;

  RailType selectedRailType = RailType::None;
  int previewGridX = -1;
  int previewGridY = -1;
  glm::vec3 railPreviewPos = glm::vec3(0);
  int railPrevierRotation = 0;

  int centralStationX, centralStationY;
  int lumberStationX, lumberStationY;
  int stoneStationX, stoneStationY;
  int ironStationX, ironStationY;

  Train train;
  std::deque<TrainState> trainHistory;
  int MAX_HISTORY = 1000;
  Order currentOrder;
  bool gameOver = false;
};
} // namespace m1
