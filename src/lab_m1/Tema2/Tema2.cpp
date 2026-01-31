#include "lab_m1/Tema2/Tema2.h"
#include "GLFW/glfw3.h"
#include "core/gpu/mesh.h"
#include "glm/ext/matrix_transform.hpp"
#include "lab_m1/Tema2/objects3D.h"
#include <iostream>
#include <regex>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;
using namespace m1;

Tema2::Tema2() {}
Tema2::~Tema2() {}

float EntryDirToRotation(Direction dir) {
  switch (dir) {
  case Direction::North:
    return -90.0f;
  case Direction::South:
    return 90.0f;
  case Direction::East:
    return 0.0f;
  case Direction::West:
    return 180.0f;
  default:
    return 0.0f;
  }
}

Direction ReverseDirection(Direction dir) {
  switch (dir) {
  case Direction::North:
    return Direction::South;
  case Direction::South:
    return Direction::North;
  case Direction::East:
    return Direction::West;
  case Direction::West:
    return Direction::East;
  default:
    return Direction::None;
  }
}

void Tema2::RenderSimpleMesh(Mesh *mesh, Shader *shader,
                             const glm::mat4 &modelMatrix, float timeFactor) {
  if (!mesh || !shader || !shader->GetProgramID()) {
    if (!mesh)
      std::cout << "Mesh is null\n";
    if (!shader)
      std::cout << "Shader is null\n";
    return;
  }

  glUseProgram(shader->program);

  int location = glGetUniformLocation(shader->program, "Model");

  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

  location = glGetUniformLocation(shader->program, "View");

  glm::mat4 viewMatrix = camera->GetViewMatrix();
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(viewMatrix));

  location = glGetUniformLocation(shader->program, "Projection");

  glm::mat4 projectionMatrix = camera->GetProjectionMatrix(
      60.0f, window->props.aspectRatio, 0.01f, 200.0f);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

  int timeLocation = glGetUniformLocation(shader->program, "timeFactor");
  glUniform1f(timeLocation, timeFactor);

  glBindVertexArray(mesh->GetBuffers()->m_VAO);
  glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()),
                 GL_UNSIGNED_INT, 0);
}

void Tema2::InitializeTrain() {
  if (!worldGrid.railTreeRoot) {
    return;
  }

  treeNode *startingRail = nullptr;
  Direction entryDir = Direction::None;

  if (worldGrid.railTreeRoot->children) {
    for (int i = 0; i < 4; ++i) {
      treeNode *child = worldGrid.railTreeRoot->children[i];
      if (!child)
        continue;

      int dr = child->row - worldGrid.railTreeRoot->row;
      int dc = child->col - worldGrid.railTreeRoot->col;

      if (abs(dr) + abs(dc) == 1) {
        startingRail = child;

        if (dr == -1)
          entryDir = Direction::South;
        else if (dr == 1)
          entryDir = Direction::North;
        else if (dc == -1)
          entryDir = Direction::East;
        else if (dc == 1)
          entryDir = Direction::West;

        break;
      }
    }
  }

  if (!startingRail) {
    return;
  }

  train.rail = startingRail;
  train.entryDir = entryDir;
  train.progress = 0.f;
  train.speed = 0.75f;
  train.waiting = false;

  float rowOffset = 0.f, colOffset = 0.f;
  switch (entryDir) {
  case Direction::North:
    rowOffset = -0.5f;
    break;
  case Direction::South:
    rowOffset = 0.5f;
    break;
  case Direction::East:
    colOffset = 0.5f;
    break;
  case Direction::West:
    colOffset = -0.5f;
    break;
  default:
    break;
  }

  train.position = glm::vec3(
      (startingRail->row + rowOffset - worldSize / 2.0f) * cellSizePixels,
      0.25f,
      (startingRail->col + colOffset - worldSize / 2.0f) * cellSizePixels);
}

void Tema2::Init() {

  currentOrder.timeLeft = 60.f;

  Shader *shader = new Shader("Tema2Shader");
  shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2",
                              "shaders", "VertexShader.glsl"),
                    GL_VERTEX_SHADER);
  shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2",
                              "shaders", "FragmentShader.glsl"),
                    GL_FRAGMENT_SHADER);
  shader->CreateAndLink();
  shaders[shader->GetName()] = shader;
  renderCameraTarget = false;
  camera = new implemented::Tema2Camera();

  float gridCenterOffset = 0.0f;
  if (worldSize % 2 == 0) {
    gridCenterOffset = -cellSizePixels / 2.0f;
  }

  float cameraHeight = cellSizePixels * 3 / 2.0f;

  float cameraX = -(worldSize / 2.0f) * cellSizePixels + gridCenterOffset;
  float cameraY = cameraHeight;
  float cameraZ = gridCenterOffset;

  glm::vec3 cameraStart(cameraX, cameraY, cameraZ);

  glm::vec3 lookAt((worldSize / 2.0f) * cellSizePixels + gridCenterOffset, 0,
                   cameraZ);

  glm::vec3 up(0, 1, 0);

  camera->Set(cameraStart, lookAt, up);

  projectionMatrix = glm::perspective(glm::radians(60.0f),
                                      window->props.aspectRatio, 0.01f, 200.0f);

  worldGrid = Grid3D(worldSize, worldSize);

  worldGrid.InitializeConnections();

  // Randomly assign a river
  riverDirection = rand() % 2;
  int riverStartX = rand() % worldSize;
  int riverStartY = rand() % worldSize;
  for (int i = 0; i < worldSize; ++i) {
    if (riverDirection == 0) {
      worldGrid.grid[riverStartX][i].type = TerrainType::Water;
    } else {
      worldGrid.grid[i][riverStartY].type = TerrainType::Water;
    }
  }

  // Randomly assign some mountains
  for (int i = 0; i < worldSize / 2; ++i) {
    int mountainX = rand() % worldSize;
    int mountainY = rand() % worldSize;
    if (worldGrid.grid[mountainY][mountainX].type == TerrainType::Plain) {
      worldGrid.grid[mountainY][mountainX].type = TerrainType::Mountain;
    }
  }

  // Randomly assign CentralStation location
  centralStationX = rand() % worldSize;
  centralStationY = rand() % worldSize;
  while (worldGrid.grid[centralStationY][centralStationX].type !=
         TerrainType::Plain) {
    centralStationX = rand() % worldSize;
    centralStationY = rand() % worldSize;
  }
  worldGrid.PlaceStation(centralStationY, centralStationX,
                         StationType::Central);
  worldGrid.railTreeRoot =
      new treeNode{centralStationY, centralStationX,      RailType::Straight,
                   Direction::None, StationType::Central, nullptr};
  // Randomly assign LumberStation location
  lumberStationX = rand() % worldSize;
  lumberStationY = rand() % worldSize;
  while (worldGrid.grid[lumberStationY][lumberStationX].type !=
             TerrainType::Plain ||
         worldGrid.grid[lumberStationY][lumberStationX].stationType !=
             StationType::None) {
    lumberStationX = rand() % worldSize;
    lumberStationY = rand() % worldSize;
  }
  worldGrid.PlaceStation(lumberStationY, lumberStationX, StationType::Lumber);

  // Randomly assign StoneStation location
  stoneStationX = rand() % worldSize;
  stoneStationY = rand() % worldSize;
  while (worldGrid.grid[stoneStationY][stoneStationX].type !=
             TerrainType::Plain ||
         worldGrid.grid[stoneStationY][stoneStationX].stationType !=
             StationType::None) {
    stoneStationX = rand() % worldSize;
    stoneStationY = rand() % worldSize;
  }
  worldGrid.PlaceStation(stoneStationY, stoneStationX, StationType::Stone);

  // Randomly assign IronStation location
  ironStationX = rand() % worldSize;
  ironStationY = rand() % worldSize;
  while (worldGrid.grid[ironStationY][ironStationX].type !=
             TerrainType::Plain ||
         worldGrid.grid[ironStationY][ironStationX].stationType !=
             StationType::None) {
    ironStationX = rand() % worldSize;
    ironStationY = rand() % worldSize;
  }
  worldGrid.PlaceStation(ironStationY, ironStationX, StationType::Iron);

  Mesh *gridCube =
      Objects3D::Create3DGridCube("gridCube", glm::vec3(0, 0, 0),
                                  (float)cellSizePixels, glm::vec3(1, 1, 1));
  AddMeshToList(gridCube);

  Mesh *riverMesh =
      Objects3D::CreateRiverMesh("riverMesh", glm::vec3(0, 0, 0),
                                 (float)cellSizePixels, 0, glm::vec3(0, 0, 1));
  AddMeshToList(riverMesh);

  Mesh *mountainMesh = Objects3D::CreateMountainMesh(
      "mountainMesh", glm::vec3(0, 0, 0), (float)cellSizePixels, 10,
      glm::vec3(0.5f, 0.35f, 0.05f));
  AddMeshToList(mountainMesh);

  Mesh *plainMesh = Objects3D::CreatePlainMesh(
      "plainMesh", glm::vec3(0, 0, 0), (float)cellSizePixels,
      (float)cellSizePixels, 1, glm::vec3(0.3f, 0.6f, 0.3f));
  AddMeshToList(plainMesh);

  Mesh *centralStationMesh = Objects3D::CreateCentralStationMesh(
      "centralStationMesh", glm::vec3(0, 0, 0), (float)cellSizePixels, 3,
      glm::vec3(0.5f, 0.5f, 0.5f));
  AddMeshToList(centralStationMesh);

  Mesh *lumberStationMesh = Objects3D::CreateLumberStationMesh(
      "lumberStationMesh", glm::vec3(0, 0, 0), (float)cellSizePixels * 1 / 2, 3,
      glm::vec3(0.55f, 0.36f, 0.20f));
  AddMeshToList(lumberStationMesh);

  Mesh *stoneStationMesh = Objects3D::CreateStoneStationMesh(
      "stoneStationMesh", glm::vec3(0, 0, 0), (float)cellSizePixels * 1 / 2, 3);
  AddMeshToList(stoneStationMesh);

  Mesh *ironStationMesh = Objects3D::CreateIronStationMesh(
      "ironStationMesh", glm::vec3(0, 0, 0), (float)cellSizePixels / 2, 3);
  AddMeshToList(ironStationMesh);

  Mesh *straightRailMesh = Objects3D::CreateStraightRailMesh(
      "straightRailMesh", glm::vec3(0, 0, 0), (float)cellSizePixels, 0.25f,
      0.25f, glm::vec3(0.2f, 0.2f, 0.25f));
  AddMeshToList(straightRailMesh);

  Mesh *bridgeMesh = Objects3D::CreateBridgeMesh(
      "bridgeMesh", glm::vec3(0, 0, 0), (float)cellSizePixels, 2.0f, 0.0f,
      glm::vec3(0.55f, 0.27f, 0.07f), glm::vec3(0.2f, 0.2f, 0.25f));
  AddMeshToList(bridgeMesh);

  Mesh *cornerRailMesh = Objects3D::CreateCornerRailMesh(
      "cornerRailMesh", glm::vec3(0, 0, 0), (float)cellSizePixels / 2, 0.25f,
      0.25f, glm::vec3(0.2f, 0.2f, 0.25f));
  AddMeshToList(cornerRailMesh);

  Mesh *mountainWithTunnelMesh = Objects3D::CreateMountainTunnelMesh(
      "mountainWithTunnelMesh", glm::vec3(0, 0, 0), (float)cellSizePixels, 10,
      glm::vec3(0.5f, 0.35f, 0.05f));
  AddMeshToList(mountainWithTunnelMesh);

  Mesh *trainMesh =
      Objects3D::CreateTrainMesh("trainMesh", glm::vec3(0.7f, 0.3f, 0.8f));
  AddMeshToList(trainMesh);

  Mesh *wagonMesh =
      Objects3D::CreateWagonMesh("wagonMesh", glm::vec3(0.8f, 0.4f, 0.2f));
  AddMeshToList(wagonMesh);
}

void Tema2::FrameStart() {
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::ivec2 resolution = window->GetResolution();
  glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::UpdateTrain(float deltaTime) {
  if (train.waiting)
    return;
  if (train.rail == nullptr) {
    return;
  }
  switch (train.rail->railType) {
  case m1::RailType::Bridge:
  case m1::RailType::Tunnel:
  case RailType::Straight: {

    if (train.rail->stationType != StationType::None) {
      switch (train.rail->stationType) {
      case StationType::Lumber: {
        for (int k = 0; k < 5; k++) {
          if (currentOrder.stuff[k] == 0) {
            if (currentOrder.timers[0] > 0.f) {
              cout << "Station is reloading lumber. Wait "
                   << currentOrder.timers[0] << " seconds.\n";
              break;
            }
            currentOrder.stuff[k] = -1;
            std::cout << "Got lumber!\n";
            currentOrder.timers[0] = 5.f;
            break;
          }
        }
        break;
      }
      case StationType::Stone: {
        for (int k = 0; k < 5; k++) {
          if (currentOrder.stuff[k] == 1) {
            if (currentOrder.timers[1] > 0.f) {
              cout << "Station is reloading stone. Wait "
                   << currentOrder.timers[1] << " seconds.\n";
              break;
            }
            currentOrder.stuff[k] = -1;
            std::cout << "Got stone!\n";
            currentOrder.timers[1] = 5.f;
            break;
          }
        }
        break;
      }
      case StationType::Iron: {
        for (int k = 0; k < 5; k++) {
          if (currentOrder.stuff[k] == 2) {
            if (currentOrder.timers[2] > 0.f) {
              cout << "Station is reloading iron. Wait "
                   << currentOrder.timers[2] << " seconds.\n";
              break;
            }
            currentOrder.stuff[k] = -1;
            std::cout << "Got iron!\n";
            currentOrder.timers[2] = 5.f;
            break;
          }
        }
        break;
      }
      case StationType::Central: {
        // Check if order is completed
        bool orderCompleted = true;
        for (int k = 0; k < 5; k++) {
          if (currentOrder.stuff[k] != -1) {
            orderCompleted = false;
            break;
          }
        }
        if (!orderCompleted) {
          std::cout << "Order not completed yet!\n";
        } else
          currentOrder.completed = true;
        break;
      }
      }

      train.entryDir = ReverseDirection(train.entryDir);

      if (train.moveDir == 1) {
        train.rail = train.rail->parent;
        train.progress = 1.0f;
      } else {
        train.rail = train.rail->children[0];
        train.progress = 0.0f;
      }
      train.moveDir = -train.moveDir;
    }

    train.progress += train.moveDir * train.speed * deltaTime;
    if (train.progress > 1.0f) {
      train.progress = 0.0f;
      treeNode *nextRail = nullptr;
      if (train.rail->children &&
          train.rail->children[0]->stationType == StationType::None) {
        nextRail = train.rail->children[0];
      } else {
        if (train.rail->children[0]->stationType != StationType::None) {
          switch (train.rail->children[0]->stationType) {
          case StationType::Lumber: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 0) {
                if (currentOrder.timers[0] > 0.f) {
                  cout << "Station is reloading lumber. Wait "
                       << currentOrder.timers[0] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got lumber!\n";
                currentOrder.timers[0] = 5.f;
                break;
              }
            }
            break;
          }
          case StationType::Stone: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 1) {
                if (currentOrder.timers[1] > 0.f) {
                  cout << "Station is reloading stone. Wait "
                       << currentOrder.timers[1] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got stone!\n";
                currentOrder.timers[1] = 5.f;
                break;
              }
            }
            break;
          }
          case StationType::Iron: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 2) {
                if (currentOrder.timers[2] > 0.f) {
                  cout << "Station is reloading iron. Wait "
                       << currentOrder.timers[2] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got iron!\n";
                currentOrder.timers[2] = 5.f;
                break;
              }
            }
            break;
          }
          }
        }
        nextRail = train.rail;
        train.moveDir = -1;
        train.entryDir = ReverseDirection(train.entryDir);
        train.progress = 1.0f;
      }
      train.rail = nextRail;
    }
    if (train.progress < 0.0f) {
      train.progress = 1.0f;

      if (train.rail->parent &&
          train.rail->parent->stationType == StationType::None) {
        train.rail = train.rail->parent;
      } else {
        if (train.rail->parent->stationType == StationType::Central) {
          // Check if order is completed
          bool orderCompleted = true;
          for (int k = 0; k < 5; k++) {
            if (currentOrder.stuff[k] != -1) {
              orderCompleted = false;
              break;
            }
          }
          if (!orderCompleted) {
            std::cout << "Order not completed yet!\n";
          } else
            currentOrder.completed = true;
        }
        train.moveDir = 1;
        train.entryDir = ReverseDirection(train.entryDir);
        train.rail = train.rail;
        train.progress = 0.0f;
      }
    }

    break;
  }
  case RailType::Corner: {
    train.progress += train.moveDir * train.speed * deltaTime;

    if (train.progress > 1.0f) {
      train.progress = 0.0f;

      treeNode *nextRail = nullptr;
      if (train.rail->children &&
          train.rail->children[0]->stationType == StationType::None)
        nextRail = train.rail->children[0];
      else {
        if (train.rail->children[0]->stationType != StationType::None) {
          switch (train.rail->children[0]->stationType) {
          case StationType::Lumber: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 0) {
                if (currentOrder.timers[0] > 0.f) {
                  cout << "Station is reloading lumber. Wait "
                       << currentOrder.timers[0] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got lumber!\n";
                currentOrder.timers[0] = 5.f;
                break;
              }
            }
            break;
          }
          case StationType::Stone: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 1) {
                if (currentOrder.timers[1] > 0.f) {
                  cout << "Station is reloading stone. Wait "
                       << currentOrder.timers[1] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got stone!\n";
                currentOrder.timers[1] = 5.f;
                break;
              }
            }
            break;
          }
          case StationType::Iron: {
            for (int k = 0; k < 5; k++) {
              if (currentOrder.stuff[k] == 2) {
                if (currentOrder.timers[2] > 0.f) {
                  cout << "Station is reloading iron. Wait "
                       << currentOrder.timers[2] << " seconds.\n";
                  break;
                }
                currentOrder.stuff[k] = -1;
                std::cout << "Got iron!\n";
                currentOrder.timers[2] = 5.f;
                break;
              }
            }
            break;
          }
          }
        }
        nextRail = train.rail;
        train.moveDir = -1;
        train.entryDir = ReverseDirection(train.entryDir);
        train.progress = 1.0f;
        break;
      }

      if (nextRail) {
        int dr = nextRail->row - train.rail->row;
        int dc = nextRail->col - train.rail->col;

        Direction newDir = Direction::None;
        if (dr == -1)
          newDir = Direction::North;
        else if (dr == 1)
          newDir = Direction::South;
        else if (dc == -1)
          newDir = Direction::West;
        else if (dc == 1)
          newDir = Direction::East;

        train.rail = nextRail;
        train.progress = 0.0f;
        train.entryDir = ReverseDirection(newDir);
      }
    }

    if (train.progress > 0.50f && train.moveDir < 0) {
      treeNode *prevRail = train.rail->parent;
      if (prevRail && prevRail->stationType == StationType::None) {
        int dr = prevRail->row - train.rail->row;
        int dc = prevRail->col - train.rail->col;

        Direction newDir = Direction::None;
        if (dr == -1)
          newDir = Direction::North;
        else if (dr == 1)
          newDir = Direction::South;
        else if (dc == -1)
          newDir = Direction::West;
        else if (dc == 1)
          newDir = Direction::East;

        train.entryDir = ReverseDirection(newDir);
      } else if (train.rail->parent->stationType == StationType::Central) {
        train.entryDir = ReverseDirection(train.entryDir);
        train.moveDir = 1;
        // Check if order is completed
        bool orderCompleted = true;
        for (int k = 0; k < 5; k++) {
          if (currentOrder.stuff[k] != -1) {
            orderCompleted = false;
            break;
          }
        }
        if (!orderCompleted) {
          std::cout << "Order not completed yet!\n";
        } else
          currentOrder.completed = true;
      }
    }

    if (train.progress < 0.0f) {
      train.progress = 0.0f;
      treeNode *prevRail = train.rail->parent;
      if (prevRail) {
        if (train.progress <= 0.0f) {
          train.rail = prevRail;
          train.progress = 1.0f;
        }
      } else {
        train.progress = 0.0f;
      }
    }

    break;
  }

  case RailType::TRail:
  case RailType::CrossRail: {
    train.progress += train.moveDir * train.speed * deltaTime;

    if (train.progress < 1.0f && train.moveDir > 0)
      break;
    if (train.progress > 1.0f && train.moveDir < 0)
      break;

    if (!train.rail->hasWaited) {
      train.waiting = true;
      train.rail->hasWaited = 1;
      break;
    }

    break;
  }
  }

  if (train.rail && train.rail->parent) {
    glm::vec3 startPos(
        (train.rail->parent->row - worldSize / 2.0f) * cellSizePixels, 0.25f,
        (train.rail->parent->col - worldSize / 2.0f) * cellSizePixels);

    glm::vec3 endPos((train.rail->row - worldSize / 2.0f) * cellSizePixels,
                     0.25f,
                     (train.rail->col - worldSize / 2.0f) * cellSizePixels);

    train.position = glm::mix(startPos, endPos, train.progress);
  }
}

void Tema2::Update(float deltaTimeSeconds) {
  // Create world grid
  for (int i = 0; i < worldSize; ++i) {
    for (int j = 0; j < worldSize; ++j) {
      glm::mat4 modelMatrix = glm::mat4(1);
      modelMatrix = glm::translate(
          modelMatrix, glm::vec3((i - worldSize / 2) * cellSizePixels, 0.0f,
                                 (j - worldSize / 2) * cellSizePixels));
      RenderMesh(meshes["gridCube"], shaders["VertexColor"], modelMatrix);
      switch (worldGrid.grid[i][j].type) {
      case TerrainType::Water: {
        RenderMesh(meshes["riverMesh"], shaders["VertexColor"], modelMatrix);
        if (worldGrid.grid[i][j].railType == RailType::Bridge) {
          if (riverDirection == 0) {
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f),
                                      glm::vec3(0, 1, 0));
          }
          RenderMesh(meshes["bridgeMesh"], shaders["VertexColor"], modelMatrix);
        }
        break;
      }
      case TerrainType::Mountain: {
        RenderMesh(meshes["plainMesh"], shaders["VertexColor"], modelMatrix);

        if (worldGrid.grid[i][j].railType == RailType::Straight) {
          modelMatrix = glm::rotate(
              modelMatrix,
              glm::radians(
                  (float)(static_cast<int>(worldGrid.grid[i][j].railDirection) *
                          90)),
              glm::vec3(0, 1, 0));
          RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                     modelMatrix);
          glm::mat4 modelMatrixTunnel =
              glm::translate(modelMatrix, glm::vec3(0, 0.01f, 0));
          RenderMesh(meshes["mountainWithTunnelMesh"], shaders["VertexColor"],
                     modelMatrixTunnel);

        } else {
          RenderMesh(meshes["mountainMesh"], shaders["VertexColor"],
                     modelMatrix);
        }
        break;
      }
      case TerrainType::Plain: {
        RenderMesh(meshes["plainMesh"], shaders["VertexColor"], modelMatrix);
        switch (worldGrid.grid[i][j].stationType) {
        case StationType::Central: {
          float timeFactor = currentOrder.timeLeft / 60.0f;
          timeFactor = glm::clamp(timeFactor, 0.0f, 1.0f);
          RenderSimpleMesh(meshes["centralStationMesh"], shaders["Tema2Shader"],
                           modelMatrix, timeFactor);
          break;
        }
        case StationType::Lumber: {
          glm::mat4 rotMatrix = glm::rotate(
              glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
          RenderMesh(meshes["lumberStationMesh"], shaders["VertexColor"],
                     modelMatrix * rotMatrix);
          break;
        }

        case StationType::Stone: {
          RenderMesh(meshes["stoneStationMesh"], shaders["VertexColor"],
                     modelMatrix);
          break;
        }
        case StationType::Iron: {
          RenderMesh(meshes["ironStationMesh"], shaders["VertexColor"],
                     modelMatrix);
          break;
        }
        default: {
          switch (worldGrid.grid[i][j].railType) {
          case RailType::Straight: {
            modelMatrix = glm::rotate(
                modelMatrix,
                glm::radians((float)(static_cast<int>(
                                         worldGrid.grid[i][j].railDirection) *
                                     90)),
                glm::vec3(0, 1, 0));
            RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            break;
          }
          case RailType::Corner: {
            modelMatrix = glm::rotate(
                modelMatrix,
                glm::radians((float)(static_cast<int>(
                                         worldGrid.grid[i][j].railDirection) *
                                     90)),
                glm::vec3(0, 1, 0));
            RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            break;
          }
          case RailType::TRail: {
            modelMatrix = glm::rotate(
                modelMatrix,
                glm::radians((float)(static_cast<int>(
                                         worldGrid.grid[i][j].railDirection) *
                                     90)),
                glm::vec3(0, 1, 0));
            RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f),
                                      glm::vec3(0, 1, 0));
            RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            break;
          }
          case RailType::CrossRail: {
            RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f),
                                      glm::vec3(0, 1, 0));
            RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                       modelMatrix);
            break;
          }
          default:
            break;
          }
        }
        }
      default:
        break;
      }
      }
    }
  }

  // If in editor mode, show and allow rail placement
  if (editorMode && selectedRailType != RailType::None && previewGridX != -1 &&
      previewGridY != -1) {

    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, railPreviewPos);

    if (worldGrid.grid[previewGridX][previewGridY].type == TerrainType::Water) {
      if (riverDirection == 0) {
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
      }
      RenderMesh(meshes["bridgeMesh"], shaders["VertexColor"], modelMatrix);
    } else {
      modelMatrix =
          glm::rotate(modelMatrix, glm::radians((float)railPrevierRotation),
                      glm::vec3(0, 1, 0));
      switch (selectedRailType) {
      case RailType::Straight:
        RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;

      case RailType::Corner:
        RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;

      case RailType::TRail:
        RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        RenderMesh(meshes["cornerRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;

      case RailType::CrossRail:
        RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        modelMatrix =
            glm::rotate(modelMatrix, glm::radians(90.f), glm::vec3(0, 1, 0));
        RenderMesh(meshes["straightRailMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;

      default:
        break;
      }
    }
  }

  if (!editorMode) {
    glm::mat4 modelMatrix = glm::mat4(1);
    float iconScale = 0.25f;
    float horizontalSpacing = 1.0f;

    for (int i = 0; i < 5; ++i) {
      if (currentOrder.stuff[i] == -1)
        continue;

      glm::mat4 modelMatrix = glm::mat4(1);

      float xOffset = (i - 2) * horizontalSpacing;
      float zOffset = 1.0f;

      modelMatrix = glm::translate(
          modelMatrix,
          glm::vec3(
              (centralStationY - worldSize / 2) * cellSizePixels + xOffset,
              cellSizePixels,
              (centralStationX - worldSize / 2) * cellSizePixels + zOffset));

      modelMatrix = glm::scale(modelMatrix, glm::vec3(iconScale));

      switch (currentOrder.stuff[i]) {
      case 0:
        RenderMesh(meshes["lumberStationMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;
      case 1:
        RenderMesh(meshes["stoneStationMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;
      case 2:
        RenderMesh(meshes["ironStationMesh"], shaders["VertexColor"],
                   modelMatrix);
        break;
      }
    }

    UpdateTrain(deltaTimeSeconds);
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, train.position);
    modelMatrix = glm::rotate(modelMatrix,
                              glm::radians(EntryDirToRotation(train.entryDir)),
                              glm::vec3(0, 1, 0));
    RenderMesh(meshes["trainMesh"], shaders["VertexColor"], modelMatrix);

    if (!train.waiting) {
      trainHistory.push_front({train.rail, train.progress, train.entryDir});
      if (trainHistory.size() > MAX_HISTORY)
        trainHistory.pop_back();
    }

    int historyIndex = 150;
    if (historyIndex < trainHistory.size()) {
      auto &wagState = trainHistory[historyIndex];

      glm::vec3 startPos(
          (wagState.rail->parent->row - worldSize / 2.0f) * cellSizePixels,
          0.25f,
          (wagState.rail->parent->col - worldSize / 2.0f) * cellSizePixels);

      glm::vec3 endPos(
          (wagState.rail->row - worldSize / 2.0f) * cellSizePixels, 0.25f,
          (wagState.rail->col - worldSize / 2.0f) * cellSizePixels);

      glm::vec3 wagonPos = glm::mix(startPos, endPos, wagState.progress);

      const float MIN_WAGON_DISTANCE = 1.65f;
      if (train.entryDir == wagState.entryDir) {
        glm::vec3 diff = wagonPos - train.position;
        float dist = glm::length(diff);
        if (dist < MIN_WAGON_DISTANCE) {
          if (dist > 0.0f) {
            wagonPos =
                train.position + glm::normalize(diff) * MIN_WAGON_DISTANCE;
          } else {
            wagonPos.z += MIN_WAGON_DISTANCE;
          }
        }
      }

      glm::mat4 wagonModel = glm::mat4(1);
      wagonModel = glm::translate(wagonModel, wagonPos);
      wagonModel = glm::rotate(
          wagonModel, glm::radians(EntryDirToRotation(wagState.entryDir)),
          glm::vec3(0, 1, 0));

      RenderMesh(meshes["wagonMesh"], shaders["VertexColor"], wagonModel);
    }

    currentOrder.timeLeft -= deltaTimeSeconds;
    currentOrder.timers[0] -= deltaTimeSeconds;
    currentOrder.timers[1] -= deltaTimeSeconds;
    currentOrder.timers[2] -= deltaTimeSeconds;
    if (currentOrder.timeLeft <= 0) {
      std::cout << "Current order expired!\n";
      gameOver = true;
    } else if (currentOrder.completed) {
      std::cout << "Current order completed! Generating new order...\n";
      currentOrder.timeLeft = 60.0f;
      currentOrder.completed = false;
      currentOrder.timers[0] = 5.0f;
      currentOrder.timers[1] = 5.0f;
      currentOrder.timers[2] = 5.0f;
      for (int i = 0; i < 5; ++i) {
        currentOrder.stuff[i] = rand() % 3;
        switch (currentOrder.stuff[i]) {
        case 0:
          std::cout << "- Lumber\n";
          break;
        case 1:
          std::cout << "- Stone\n";
          break;
        case 2:
          std::cout << "- Iron\n";
          break;
        default:
          break;
        }
      }
    }
  }
}

void Tema2::FrameEnd() {
  // DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Tema2::RenderMesh(Mesh *mesh, Shader *shader,
                       const glm::mat4 &modelMatrix) {
  if (!mesh || !shader || !shader->program)
    return;

  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(modelMatrix));

  mesh->Render();
}

void Tema2::OnInputUpdate(float deltaTime, int mods) {
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    if (window->KeyHold(GLFW_KEY_W)) {
      camera->TranslateForward(deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_A)) {
      camera->TranslateRight(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_S)) {
      camera->TranslateForward(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_D)) {
      camera->TranslateRight(deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_Q)) {
      camera->TranslateUpward(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_E)) {
      camera->TranslateUpward(deltaTime);
    }
  }
}

void Tema2::OnKeyPress(int key, int mods) {
  if (editorMode)
    switch (key) {
    case GLFW_KEY_0: {
      selectedRailType = RailType::None;
      cout << "Rail placement mode OFF" << endl;
      break;
    }
    case GLFW_KEY_1: {
      selectedRailType = RailType::Straight;
      cout << "Rail placement mode ON: Straight Rail selected" << endl;
      break;
    }
    case GLFW_KEY_2: {
      selectedRailType = RailType::Corner;
      cout << "Rail placement mode ON: Corner Rail selected" << endl;
      break;
    }
    case GLFW_KEY_3: {
      selectedRailType = RailType::TRail;
      cout << "Rail placement mode ON: T Rail selected" << endl;
      break;
    }
    case GLFW_KEY_4: {
      selectedRailType = RailType::CrossRail;
      cout << "Rail placement mode ON: Cross Rail selected" << endl;
      break;
    }
    case GLFW_KEY_R: {
      if (selectedRailType != RailType::None) {
        railPrevierRotation = (railPrevierRotation + 90) % 360;
      }
      break;
    }
    default:
      break;
    }
  if (key == GLFW_KEY_ENTER) {
    bool ready = true;
    worldGrid.AddStationToTree(lumberStationY, lumberStationX);
    worldGrid.AddStationToTree(stoneStationY, stoneStationX);
    worldGrid.AddStationToTree(ironStationY, ironStationX);

    if (!worldGrid.IsCellInTree(worldGrid.railTreeRoot, lumberStationY,
                                lumberStationX)) {
      ready = false;
    }
    if (!worldGrid.IsCellInTree(worldGrid.railTreeRoot, stoneStationY,
                                stoneStationX)) {
      ready = false;
    }
    if (!worldGrid.IsCellInTree(worldGrid.railTreeRoot, ironStationY,
                                ironStationX)) {
      ready = false;
    }
    if (ready) {
      editorMode = false;
      std::cout << "Exiting editor mode. Simulation started!" << std::endl;
      InitializeTrain();
      std::cout << "Current order is:\n";
      currentOrder.timers[0] = 5.0f;
      currentOrder.timers[1] = 5.0f;
      currentOrder.timers[2] = 5.0f;
      currentOrder.completed = false;
      for (int i = 0; i < 5; ++i) {
        currentOrder.stuff[i] = rand() % 3;
        switch (currentOrder.stuff[i]) {
        case 0:
          std::cout << "- Lumber\n";
          break;
        case 1:
          std::cout << "- Stone\n";
          break;
        case 2:
          std::cout << "- Iron\n";
          break;
        default:
          break;
        }
      }
      currentOrder.timeLeft = 60.0f;

    } else {
      cout << "Cannot start simulation. Some stations are not connected!"
           << endl;
    }
  }

  if (!editorMode) {
    if (key == GLFW_KEY_UP) {
      if (train.moveDir == -1)
        train.entryDir = ReverseDirection(train.entryDir);
      train.moveDir = 1;
    }

    if (key == GLFW_KEY_DOWN) {
      if (train.moveDir == 1)
        train.entryDir = ReverseDirection(train.entryDir);
      train.moveDir = -1;
    }
  }
  if (train.waiting) {
    int chosenDir = 3;

    switch (key) {
    case GLFW_KEY_W:
      chosenDir = -1;
      break;
    case GLFW_KEY_A:
      chosenDir = 0;
      break;
    case GLFW_KEY_D:
      chosenDir = 2;
      break;

    case GLFW_KEY_S:
      chosenDir = 1;
      break;
    }

    if (chosenDir != 3 && train.rail->children) {
      treeNode *nextRail = nullptr;

      std::vector<treeNode *> candidates;

      for (int i = 0; i < 4; ++i) {
        if (train.rail->children[i])
          candidates.push_back(train.rail->children[i]);
      }

      if (train.rail->parent)
        candidates.push_back(train.rail->parent);

      for (treeNode *nextRail : candidates) {
        int dr = nextRail->row - train.rail->row;
        int dc = nextRail->col - train.rail->col;

        Direction absDir = Direction::None;
        if (dr == -1)
          absDir = Direction::North;
        else if (dr == 1)
          absDir = Direction::South;
        else if (dc == -1)
          absDir = Direction::West;
        else if (dc == 1)
          absDir = Direction::East;

        int relative = -1;

        switch (train.entryDir) {
        case Direction::North:
          if (absDir == Direction::West)
            relative = 0;
          else if (absDir == Direction::North)
            relative = 1;
          else if (absDir == Direction::East)
            relative = 2;
          break;
        case Direction::East:
          if (absDir == Direction::North)
            relative = 0;
          else if (absDir == Direction::East)
            relative = 1;
          else if (absDir == Direction::South)
            relative = 2;
          break;
        case Direction::South:
          if (absDir == Direction::East)
            relative = 0;
          else if (absDir == Direction::South)
            relative = 1;
          else if (absDir == Direction::West)
            relative = 2;
          break;
        case Direction::West:
          if (absDir == Direction::South)
            relative = 0;
          else if (absDir == Direction::West)
            relative = 1;
          else if (absDir == Direction::North)
            relative = 2;
          break;
        default:
          break;
        }

        if (relative == chosenDir) {
          train.rail->hasWaited = false;
          if (train.rail->parent == nextRail)
            train.moveDir = -1;
          else
            train.moveDir = 1;
          train.entryDir = ReverseDirection(absDir);
          train.rail = nextRail;
          if (train.moveDir == -1) {
            train.progress = 1.0f;
          } else
            train.progress = 0.0f;
          train.waiting = false;
          break;
        }
      }
    }
  }
}
void Tema2::OnKeyRelease(int key, int mods) {
  // Add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
  // Add mouse move event

  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    float sensivityOX = 0.001f;
    float sensivityOY = 0.001f;
    camera->RotateFirstPerson_OX(-deltaY * sensivityOX);
    camera->RotateFirstPerson_OY(-deltaX * sensivityOY);
  }

  if (selectedRailType != RailType::None) {

    glm::ivec2 res = window->GetResolution();

    // Here we do some pixels black magic

    float x = (2.0f * mouseX) / res.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / res.y;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::normalize(
        glm::vec3(glm::inverse(camera->GetViewMatrix()) * ray_eye));

    glm::vec3 camPos = camera->position;

    if (fabs(ray_world.y) < 0.0001f)
      return;

    float t = -camPos.y / ray_world.y;
    glm::vec3 hitPos = camPos + t * ray_world;

    float localX = hitPos.x + worldSize / 2.0f * cellSizePixels;
    float localZ = hitPos.z + worldSize / 2.0f * cellSizePixels;

    int gridX = (int)round(localX / cellSizePixels);
    int gridY = (int)round(localZ / cellSizePixels);

    if (gridX < 0 || gridX >= worldSize || gridY < 0 || gridY >= worldSize)
      return;

    previewGridX = gridX;
    previewGridY = gridY;

    float snappedX = (gridX - worldSize / 2.0f) * cellSizePixels;
    float snappedZ = (gridY - worldSize / 2.0f) * cellSizePixels;

    railPreviewPos = glm::vec3(snappedX, 0.0f, snappedZ);
  }
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
  // Add mouse button press event
  if (button == 1 && editorMode) {
    if (selectedRailType != RailType::None &&
        worldGrid.IsValidCell(previewGridX, previewGridY) &&
        worldGrid.grid[previewGridX][previewGridY].railType == RailType::None &&
        worldGrid.grid[previewGridX][previewGridY].stationType ==
            StationType::None) {
      Direction direction = Direction::None;
      switch (railPrevierRotation) {
      case 90:
        direction = Direction::North;
        break;
      case 180:
        direction = Direction::East;
        break;
      case 270:
        direction = Direction::South;
        break;
      case 0:
        direction = Direction::West;
        break;
      default:
        direction = Direction::None;
        break;
      }

      if (worldGrid.grid[previewGridX][previewGridY].type ==
          TerrainType::Water) {
        if (riverDirection == 0)
          direction = Direction::North;
        else
          direction = Direction::East;
        worldGrid.PlaceRail(previewGridX, previewGridY, RailType::Bridge,
                            direction);
        return;
      }

      worldGrid.PlaceRail(previewGridX, previewGridY, selectedRailType,
                          direction);
    }
  }
  if (button == 2 && editorMode) {
    if (worldGrid.IsValidCell(previewGridX, previewGridY) &&
        worldGrid.grid[previewGridX][previewGridY].railType != RailType::None) {
      worldGrid.RemoveRail(previewGridX, previewGridY);
    }
  }
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
  // Add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Tema2::OnWindowResize(int width, int height) {}
