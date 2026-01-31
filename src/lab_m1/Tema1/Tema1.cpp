#include "lab_m1/Tema1/Tema1.h"

#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "GLFW/glfw3.h"
#include "components/text_renderer.h"
#include "core/gpu/mesh.h"
#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "lab_m1/Tema1/object.h"
#include "lab_m1/Tema1/transform.h"
#include "utils/gl_utils.h"
#include "utils/math_utils.h"

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

Tema1::Tema1() {}

Tema1::~Tema1() {}

void Tema1::Init() {
  resolution = window->GetResolution();
  auto camera = GetSceneCamera();
  camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f,
                          400);
  camera->SetPosition(glm::vec3(0, 0, 50));
  camera->SetRotation(glm::vec3(0, 0, 0));
  camera->Update();
  GetCameraInput()->SetActive(false);

  glm::vec3 corner = glm::vec3(1, 1, 0);
  float squareSide = 100;

  speed = glm::vec2(400.0f, 400.f);
  angle = glm::radians(45.0f);

  ballUpdate = glm::vec2(0);

  text =
      new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);

  text->Load("../assets/fonts/Hack-Bold.ttf", 50);

  shipComponents = 4;
  editorComponentsSize = (float)resolution.y / shipComponents;
  componentLeftLength = 50;

  inGame = 0;

  livesLeft = 3;

  score = 0;

  mousePos.x = 0;
  mousePos.y = 0;

  gridSize.x = (resolution.x * 5 / 6 - 20) / 60;
  gridSize.y = (resolution.y * 5 / 6 - 20) / 60;

  grid = Grid(gridSize.x, gridSize.y, 10);

  brickLen = resolution.x / 10;
  brickWidth = 50;

  bricks = std::vector<std::vector<int>>(resolution.x / brickLen,
                                         std::vector<int>(4));

  bricksLeft = resolution.x / brickLen * 4;

  bricksTimer = std::vector<std::vector<float>>(resolution.x / brickLen,
                                                std::vector<float>(4));

  for (int i = 0; i < bricks.size(); ++i) {
    for (int j = 0; j < bricks[i].size(); ++j) {
      bricks[i][j] = rand() % 4;
    }
  }

  componentSelected = ComponentType::None;

  ComponentTypes.push_back(ComponentType::Bumper);
  ComponentTypes.push_back(ComponentType::Cannon);
  ComponentTypes.push_back(ComponentType::Engine);
  ComponentTypes.push_back(ComponentType::Block);

  translateX = 0;
  translateY = 0;

  started = 0;
  start = 0;

  shakeTime = 0.0f;
  shakeDuration = 0.25f;
  shakeStrength = 15.0f;
  shakeOffset = glm::vec2(0);

  shipUpdate = glm::vec2(0);

  componentsX = resolution.x / 6;
  componentsY = 0;

  lost = 0;
  won = 0;

  Mesh *border = object::CreateRectangle("border", corner, resolution.x - 1,
                                         resolution.y - 1, glm::vec3(1, 0, 0));
  AddMeshToList(border);

  Mesh *componentsBorder =
      object::CreateRectangle("componentsBorder", corner, componentsX,
                              editorComponentsSize, glm::vec3(1, 0, 0));
  AddMeshToList(componentsBorder);

  Mesh *componentsLeft = object::CreateSquare(
      "componentsLeft", glm::vec3(resolution.x / 5, resolution.y * 6 / 7, 0),
      componentLeftLength, glm::vec3(0, 1, 0), true);
  AddMeshToList(componentsLeft);

  Mesh *startButton = object::CreateRectangle(
      "startButton", glm::vec3(resolution.x - 200, resolution.y * 5 / 6, -1),
      150, 100, glm::vec3(0, 1, 0), true);
  AddMeshToList(startButton);

  Mesh *startNotButton = object::CreateRectangle(
      "startNotButton", glm::vec3(resolution.x - 200, resolution.y * 5 / 6, -1),
      150, 100, glm::vec3(1, 0, 0), true);
  AddMeshToList(startNotButton);

  Mesh *gridBorder = object::CreateRectangle(
      "gridBorder", glm::vec3(resolution.x / 6 + 10, 10, 0),
      resolution.x * 5 / 6 - 20, resolution.y * 5 / 6 - 20, glm::vec3(0, 0, 1));
  AddMeshToList(gridBorder);

  Mesh *gridSquare = object::CreateSquare(
      "gridSquare", glm::vec3(resolution.x / 6 + 25, 35, 0), 50,
      glm::vec3(0, 0, 1));
  AddMeshToList(gridSquare);

  Mesh *shipBlock = object::CreateSquare(
      "shipBlock",
      corner + glm::vec3(resolution.x / 12 - 30, editorComponentsSize / 2 - 30,
                         0.01),
      60, glm::vec3(0.5, 0.5, 0.5), true);
  AddMeshToList(shipBlock);

  Mesh *shipEngine = object::CreateEngine(
      "shipEngine",
      corner + glm::vec3(resolution.x / 12 - 30,
                         editorComponentsSize * 3 / 2 - 30, 0.01),
      60);
  AddMeshToList(shipEngine);

  Mesh *shipCannon = object::CreateCannon(
      "shipCannon",
      corner + glm::vec3(resolution.x / 12 - 30, editorComponentsSize * 2 + 10,
                         0.01),
      60);
  AddMeshToList(shipCannon);

  Mesh *shipBumper = object::CreateBumper(
      "shipBumper",
      corner + glm::vec3(resolution.x / 12 - 30,
                         editorComponentsSize * 6 / 2 + 20, 0.01),
      60);
  AddMeshToList(shipBumper);

  Mesh *yellowBrick = object::CreateRectangle(
      "yellowBrick", glm::vec3(0, resolution.y - brickWidth - 50, 0), brickLen,
      brickWidth, glm::vec3(1, 1, 0), true);
  AddMeshToList(yellowBrick);

  Mesh *redBrick = object::CreateRectangle(
      "redBrick", glm::vec3(0, resolution.y - brickWidth - 50, 0), brickLen,
      brickWidth, glm::vec3(1, 0, 0), true);
  AddMeshToList(redBrick);

  Mesh *greenBrick = object::CreateRectangle(
      "greenBrick", glm::vec3(0, resolution.y - brickWidth - 50, 0), brickLen,
      brickWidth, glm::vec3(0, 1, 0), true);
  AddMeshToList(greenBrick);

  Mesh *orangeBrick = object::CreateRectangle(
      "orangeBrick", glm::vec3(0, resolution.y - brickWidth - 50, 0), brickLen,
      brickWidth, glm::vec3(1, 0.647, 0), true);
  AddMeshToList(orangeBrick);

  Mesh *borderBrick = object::CreateRectangle(
      "borderBrick", glm::vec3(0, resolution.y - brickWidth - 50, 0), brickLen,
      brickWidth, glm::vec3(0, 0, 0), false);
  AddMeshToList(borderBrick);

  Mesh *ball = object::CreateCircle("ball", corner, 5, glm::vec3(1, 1, 1));
  AddMeshToList(ball);

  Mesh *particle =
      object::CreateCircle("particle", corner, 5, glm::vec3(1, 0, 0));
  AddMeshToList(particle);

  Mesh *testBlock =
      object::CreateSquare("testBlock", corner, 60, glm::vec3(1, 1, 1), true);
  AddMeshToList(testBlock);

  Mesh *heart = object::CreateHeart(
      "heart", glm::vec3(resolution.x - 20, resolution.y - 20, 0), 20,
      glm::vec3(1, 0, 0));
  AddMeshToList(heart);
}

void Tema1::FrameStart() {
  // Clears the color buffer (using the previously set color) and depth buffer
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Sets the screen area where to draw
  glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);
}

void Tema1::Update(float deltaTimeSeconds) {
  if (inGame == 0)
    UpdateEditor(deltaTimeSeconds);
  else if (!lost && !won)
    UpdateGame(deltaTimeSeconds);
  else {
    UpdateEnd(deltaTimeSeconds);
  }
}

void Tema1::UpdateEditor(float deltaTimeSeconds) {
  // outerBorder
  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["border"], shaders["VertexColor"], modelMatrix);

  // componentsBorder

  for (int i = 0; i < shipComponents; ++i) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform::Translate(0, i * editorComponentsSize);
    RenderMesh2D(meshes["componentsBorder"], shaders["VertexColor"],
                 modelMatrix);
  }

  // componentsLeft
  for (int i = 0; i < grid.componentsNr; ++i) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform::Translate(i * 1.5 * componentLeftLength, 0);
    RenderMesh2D(meshes["componentsLeft"], shaders["VertexColor"], modelMatrix);
  }

  // gridBorder
  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["gridBorder"], shaders["VertexColor"], modelMatrix);

  // gridSquares
  grid.ok = true;
  for (int i = 0; i < gridSize.x; ++i) {
    for (int j = 0; j < gridSize.y; ++j) {
      if (grid.isOverlapped(i, j)) {
        grid.ok = false;
        // cout << grid.grid[i][j].nrOc << " " << (grid.grid[i][j].nrOc <= 1) <<
        // "\n";
      }
      if (grid.IsValidCell(i, j)) {
        modelMatrix = glm::mat3(1);
        modelMatrix *= transform::Translate(i * 60, j * 60);
        RenderMesh2D(meshes["gridSquare"], shaders["VertexColor"], modelMatrix);
      }
    }
  }

  // ship
  maxX = 0;
  minX = grid.rows;

  maxY = 0;
  minY = grid.columns;

  for (int i = 0; i < grid.rows; ++i) {
    for (int j = 0; j < grid.columns; ++j) {
      switch (grid.GetComponent(i, j)) {
      case ComponentType::Block:
        if (minX > i)
          minX = i;
        if (maxX < i)
          maxX = i;
        if (minY > j)
          minY = j;
        if (maxY < j)
          maxY = j;
        modelMatrix = glm::mat3(1);
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 60 - editorComponentsSize / 2);
        RenderMesh2D(meshes["shipBlock"], shaders["VertexColor"], modelMatrix);
        break;

      case ComponentType::Cannon:
        if (minX > i)
          minX = i;
        if (maxX < i)
          maxX = i;
        if (minY > j)
          minY = j;
        if (maxY < j)
          maxY = j;
        modelMatrix = glm::mat3(1);
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 20 - editorComponentsSize * 2);
        RenderMesh2D(meshes["shipCannon"], shaders["VertexColor"], modelMatrix);
        break;

      case ComponentType::Engine:
        if (minX > i)
          minX = i;
        if (maxX < i)
          maxX = i;
        if (minY > j)
          minY = j;
        if (maxY < j)
          maxY = j;

        modelMatrix = glm::mat3(1);
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 60 - editorComponentsSize * 3 / 2);
        RenderMesh2D(meshes["shipEngine"], shaders["VertexColor"], modelMatrix);
        break;

      case ComponentType::Bumper:
        if (minX > i - 1)
          minX = i - 1;
        if (maxX < i + 1)
          maxX = i + 1;
        if (minY > j)
          minY = j;
        if (maxY < j)
          maxY = j;
        modelMatrix = glm::mat3(1);
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 10 - editorComponentsSize * 3);
        RenderMesh2D(meshes["shipBumper"], shaders["VertexColor"], modelMatrix);
        break;
      default:
        break;
      }
    }
  }

  if (!grid.checkConnected())
    grid.ok = grid.checkConnected();

  // startButton
  modelMatrix = glm::mat3(1);

  if (grid.ok)
    RenderMesh2D(meshes["startButton"], shaders["VertexColor"], modelMatrix);
  else
    RenderMesh2D(meshes["startNotButton"], shaders["VertexColor"], modelMatrix);
  glDisable(GL_DEPTH_TEST);
  text->RenderText("START", resolution.x - 200, 50, 1);
  glEnable(GL_DEPTH_TEST);
  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["shipBlock"], shaders["VertexColor"], modelMatrix);

  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["shipEngine"], shaders["VertexColor"], modelMatrix);

  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["shipCannon"], shaders["VertexColor"], modelMatrix);

  modelMatrix = glm::mat3(1);
  RenderMesh2D(meshes["shipBumper"], shaders["VertexColor"], modelMatrix);

  // M Drag an Drop
  modelMatrix = glm::mat3(1);

  // printf("%d\n", static_cast<int>(componentSelected));

  switch (componentSelected) {
  case ComponentType::Block:
    modelMatrix *= transform::Translate(mousePos.x - resolution.x / 12,
                                        mousePos.y - editorComponentsSize / 2);
    modelMatrix *= transform::Translate(translateX, translateY);
    RenderMesh2D(meshes["shipBlock"], shaders["VertexColor"], modelMatrix);
    break;
  case ComponentType::Engine:
    modelMatrix *=
        transform::Translate(mousePos.x - resolution.x / 12,
                             mousePos.y - editorComponentsSize * 3 / 2);
    modelMatrix *= transform::Translate(translateX, translateY);
    RenderMesh2D(meshes["shipEngine"], shaders["VertexColor"], modelMatrix);
    break;
  case ComponentType::Cannon:
    modelMatrix *=
        transform::Translate(mousePos.x - resolution.x / 12,
                             mousePos.y - editorComponentsSize * 2 - 40);
    modelMatrix *= transform::Translate(translateX, translateY);
    RenderMesh2D(meshes["shipCannon"], shaders["VertexColor"], modelMatrix);
    break;
  case ComponentType::Bumper:
    modelMatrix *=
        transform::Translate(mousePos.x - resolution.x / 12,
                             mousePos.y - editorComponentsSize * 3 - 40);
    modelMatrix *= transform::Translate(translateX, translateY);
    RenderMesh2D(meshes["shipBumper"], shaders["VertexColor"], modelMatrix);
    break;
  default:
    break;
  }
}

void Tema1::UpdateGame(float deltaTimeSeconds) {
  int centerX = (maxX + minX) / 2 + 3 + (maxX + minX) % 2;
  int centerWorld = resolution.x / 120;

  if (started == 0) {
    speed = glm::vec2(0);
    if (start) {
      speed = glm::vec2(300, 300);
      started = 1;
    }
  } else {
    speed.x += glm::sign(speed.x) * 0.01;
    speed.y += glm::sign(speed.y) * 0.01;
  }
  std::string scoreStr = "Score: " + to_string(score);
  text->RenderText(scoreStr, 0, 10, 1.0f);

  std::string enemyStr = "Foes left: " + to_string(bricksLeft);
  text->RenderText(enemyStr, 500, 10, 1.0f);

  for (int i = 0; i < livesLeft; ++i) {
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
    modelMatrix *= transform::Translate(-i * 40, 0);
    RenderMesh2D(meshes["heart"], shaders["VertexColor"], modelMatrix);
  }

  if (shakeTime > 0) {
    shakeTime -= deltaTimeSeconds;

    float power = (shakeTime / shakeDuration); // scade treptat
    shakeOffset.x = (rand() % 200 - 100) / 100.0f * shakeStrength * power;
    shakeOffset.y = (rand() % 200 - 100) / 100.0f * shakeStrength * power;
  } else {
    shakeOffset = glm::vec2(0);
  }

  prevBallLocation = ballLocation;
  ballUpdate.x += speed.x * cos(angle) * deltaTimeSeconds;
  ballUpdate.y += speed.y * sin(angle) * deltaTimeSeconds;

  //  ball render
  ballLocation =
      glm::vec2(centerWorld * 60 + 30, ((maxY - minY) + 2) * 60 + 130);
  ballLocation += ballUpdate;
  if (ballLocation.x < 0 || ballLocation.x > resolution.x)
    speed.x = -speed.x;
  if (ballLocation.y < 0 || ballLocation.y > resolution.y) {
    livesLeft--;
    if (livesLeft == 0)
      lost = 1;
    else {
      shakeTime = shakeDuration;
      ballUpdate = glm::vec2(0);
      start = 0;
      started = 0;
      shipUpdate = glm::vec2(0, 0);
    }
  }

  // bricks rendering
  for (int i = 0; i < bricks.size(); ++i) {
    for (int j = 0; j < bricks[i].size(); ++j) {
      modelMatrix = glm::mat3(1);
      modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
      modelMatrix *= transform::Translate(brickLen * i, -brickWidth * j);
      if (bricks[i][j] >= 0) {
        if (resolution.y - brickWidth - 50 - brickWidth * j <= ballLocation.y &&
            ballLocation.y <= resolution.y - 50 - brickWidth * j &&
            brickLen * i <= ballLocation.x &&
            ballLocation.x <= brickLen * i + brickLen) {
          // am lovit brick
          score++;

          speed = -speed;
          if (bricks[i][j] > 0) {
            bricksTimer[i][j] = 0.2f;
          } else if (bricks[i][j] == 0) {
            bricksLeft--;
            if (bricksLeft == 0)
              won = 1;
            for (int p = 0; p < 20; p++) {
              Particle part;
              part.pos = glm::vec2(brickLen * i + brickLen / 2,
                                   resolution.y - brickWidth - 50 -
                                       brickWidth * j + brickWidth / 2);

              float particleAngle = (rand() % 360) * 3.14159f / 180.0f;
              float particleSpeed = 200 + rand() % 150;

              part.vel = glm::vec2(cos(particleAngle), sin(particleAngle)) *
                         particleSpeed;
              part.life = 0.67f;

              particles.push_back(part);
            }
          }

          bricks[i][j]--;
        }
      }
      if (bricksTimer[i][j] > 0.0f) {
        float shakeDuration = 0.3f;
        float shakeMagnitude = 5.0f;
        float t = bricksTimer[i][j] / shakeDuration;
        float offsetX = shakeMagnitude * sin(t * 10.0f * M_PI);
        float offsetY = shakeMagnitude * sin(t * 15.0f * M_PI);

        modelMatrix *= transform::Translate(offsetX, offsetY);

        bricksTimer[i][j] -= deltaTimeSeconds;
        if (bricksTimer[i][j] < 0)
          bricksTimer[i][j] = 0;
      }

      switch (bricks[i][j]) {
      case 0:
        RenderMesh2D(meshes["borderBrick"], shaders["VertexColor"],
                     modelMatrix);
        RenderMesh2D(meshes["redBrick"], shaders["VertexColor"], modelMatrix);
        break;

      case 1:
        RenderMesh2D(meshes["borderBrick"], shaders["VertexColor"],
                     modelMatrix);
        RenderMesh2D(meshes["yellowBrick"], shaders["VertexColor"],
                     modelMatrix);
        break;

      case 2:
        RenderMesh2D(meshes["borderBrick"], shaders["VertexColor"],
                     modelMatrix);
        RenderMesh2D(meshes["orangeBrick"], shaders["VertexColor"],
                     modelMatrix);
        break;

      case 3:
        RenderMesh2D(meshes["borderBrick"], shaders["VertexColor"],
                     modelMatrix);
        RenderMesh2D(meshes["greenBrick"], shaders["VertexColor"], modelMatrix);
        break;
      default:
        break;
      }
    }
  }
  for (int p = 0; p < particles.size();) {
    particles[p].life -= deltaTimeSeconds;

    if (particles[p].life <= 0) {
      particles.erase(particles.begin() + p);
      continue;
    }

    particles[p].pos += particles[p].vel * deltaTimeSeconds;

    particles[p].vel.y -= 300 * deltaTimeSeconds;
    modelMatrix = glm::mat3(1);
    modelMatrix *= transform::Translate(particles[p].pos.x, particles[p].pos.y);
    modelMatrix *= transform::Scale(0.3f, 0.3f);
    modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
    RenderMesh2D(meshes["particle"], shaders["VertexColor"], modelMatrix);

    ++p;
  }

  modelMatrix = glm::mat3(1);
  modelMatrix *= transform::Translate(ballLocation.x, ballLocation.y);
  modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
  RenderMesh2D(meshes["ball"], shaders["VertexColor"], modelMatrix);

  int ballLeft = ballLocation.x - 5;
  int ballRight = ballLocation.x + 5;
  int ballTop = ballLocation.y - 5;
  int ballBottom = ballLocation.y + 5;

  // ship moving
  glm::vec2 componentPos = glm::vec2(0);
  shipLocation = glm::vec2(0, -60 * (minY - 1) - 10);
  shipLocation.x += (centerWorld - centerX) * 60 + 15;
  if ((minX + maxX) % 2 == 1)
    shipLocation.x += 30;
  shipLocation.x += shipUpdate.x;

  int blockLeft = 0;
  int blockRight = 0;
  int blockBottom = 0;
  int blockTop = 0;
  int singleBlockHeight = 60 * 0.75f;

  for (int i = 0; i < grid.rows; ++i) {
    for (int j = 0; j < grid.columns; ++j) {
      modelMatrix = glm::mat3(1);
      modelMatrix *= transform::Translate(shipLocation.x, shipLocation.y);
      modelMatrix *=
          transform::Translate((minX + maxX) * 60 / 2, (minY + maxY) * 60 / 2);
      modelMatrix *= transform::Scale(0.75f, 0.75f);
      modelMatrix *= transform::Translate(-(minX + maxX) * 60 / 2,
                                          -(minY + maxY) * 60 / 2);
      modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);

      switch (grid.GetComponent(i, j)) {
      case ComponentType::Block: {
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 60 - editorComponentsSize / 2);
        RenderMesh2D(meshes["shipBlock"], shaders["VertexColor"], modelMatrix);

        componentPos.x = modelMatrix[2][0] + resolution.x / 12 - 49;
        componentPos.y = modelMatrix[2][1] + editorComponentsSize / 2 - 45;

        blockLeft = componentPos.x;
        blockRight = componentPos.x + 60 * 0.75f;
        blockBottom = componentPos.y;
        blockTop = componentPos.y + 60 * 0.75f;
        break;
      }

      case ComponentType::Cannon: {
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 20 - editorComponentsSize * 2);
        modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
        RenderMesh2D(meshes["shipCannon"], shaders["VertexColor"], modelMatrix);

        componentPos.x = modelMatrix[2][0] + resolution.x / 12 - 49;
        componentPos.y = modelMatrix[2][1] + editorComponentsSize * 2 - 82;

        blockLeft = componentPos.x;
        blockRight = componentPos.x + 60 * 0.75f;
        blockBottom = componentPos.y;
        blockTop = componentPos.y + 60 * 0.75f * 3 - 25;
        break;
      }

      case ComponentType::Engine: {
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 60 - editorComponentsSize * 3 / 2);
        modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
        RenderMesh2D(meshes["shipEngine"], shaders["VertexColor"], modelMatrix);

        componentPos.x = modelMatrix[2][0] + resolution.x / 12 - 49;
        componentPos.y = modelMatrix[2][1] + editorComponentsSize * 3 / 2 - 100;

        blockLeft = componentPos.x;
        blockRight = componentPos.x + 60 * 0.75f;
        blockBottom = componentPos.y - 60 * 0.75f;
        blockTop = componentPos.y + 60 * 0.75f;
        break;
      }

      case ComponentType::Bumper: {
        modelMatrix *=
            transform::Translate(i * 60 + resolution.x / 12 + 50,
                                 j * 60 + 10 - editorComponentsSize * 3);
        modelMatrix *= transform::Translate(shakeOffset.x, shakeOffset.y);
        RenderMesh2D(meshes["shipBumper"], shaders["VertexColor"], modelMatrix);

        componentPos.x = modelMatrix[2][0] + resolution.x / 12 - 49;
        componentPos.y = modelMatrix[2][1] + editorComponentsSize * 3 - 120;

        blockLeft = componentPos.x - 60 * 0.75f;
        blockRight = componentPos.x + 60 * 2 * 0.75f;
        blockBottom = componentPos.y;
        blockTop = componentPos.y + 60 * 2 * 0.75f;

        break;
      }

      default:
        continue;
      }

      glm::vec2 nextBall = ballLocation + speed * deltaTimeSeconds;
      float radius = 5.0f;

      float overlapLeft = nextBall.x + radius - blockLeft;
      float overlapRight = blockRight - (nextBall.x - radius);
      float overlapBottom = nextBall.y + radius - blockBottom;
      float overlapTop = blockTop - (nextBall.y - radius);

      if (overlapLeft > 0 && overlapRight > 0 && overlapTop > 0 &&
          overlapBottom > 0) {
        float minOverlap =
            std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
        if (minOverlap == overlapLeft) {
          speed.x = -abs(speed.x);
          ballLocation.x = blockLeft - radius;
        } else if (minOverlap == overlapRight) {
          speed.x = abs(speed.x);
          ballLocation.x = blockRight + radius;
        } else if (minOverlap == overlapBottom) {
          speed.y = -abs(speed.y);
          ballLocation.y = blockBottom - radius;
        } else if (minOverlap == overlapTop) {
          ballLocation.y = blockTop + radius;
          // Predict next position to avoid tunneling
          // Predict next position to avoid tunneling
          glm::vec2 nextBall = ballLocation + speed * deltaTimeSeconds;
          float radius = 5.0f;

          // Compute overlaps for collision resolution
          float overlapLeft = nextBall.x + radius - blockLeft;
          float overlapRight = blockRight - (nextBall.x - radius);
          float overlapBottom = nextBall.y + radius - blockBottom;
          float overlapTop = blockTop - (nextBall.y - radius);

          // Check if collision occurs
          if (overlapLeft > 0 && overlapRight > 0 && overlapTop > 0 &&
              overlapBottom > 0) {
            float minOverlap = std::min(
                {overlapLeft, overlapRight, overlapTop, overlapBottom});
            float speedMag = glm::length(speed);

            if (minOverlap == overlapLeft) {
              speed.x = -fabs(speed.x);
              ballLocation.x = blockLeft - radius;
            } else if (minOverlap == overlapRight) {
              speed.x = fabs(speed.x);
              ballLocation.x = blockRight + radius;
            } else if (minOverlap == overlapBottom) {
              speed.y = -fabs(speed.y);
              ballLocation.y = blockBottom - radius;
            } else if (minOverlap == overlapTop) {
              float cellWorldLeft = minX * 60.0f;
              float cellWorldRight = (maxX + 1) * 60.0f;

              float shipWorldLeft =
                  cellWorldLeft + resolution.x / 12.0f + 50.0f + shipLocation.x;
              float shipWorldRight = cellWorldRight + resolution.x / 12.0f +
                                     50.0f + shipLocation.x;

              float shipCenterX = (shipWorldLeft + shipWorldRight) * 0.5f;
              float shipHalfWidth = (shipWorldRight - shipWorldLeft) * 0.5f;
              if (shipHalfWidth < 1.0f)
                shipHalfWidth = 1.0f;

              float relativeX = (ballLocation.x - shipCenterX) / shipHalfWidth;
              relativeX = glm::clamp(relativeX, -1.0f, 1.0f);

              float maxAngle = glm::radians(75.0f);
              float incomingSign = glm::sign(speed.x) * 1.0f;

              float bounceAngle = relativeX * maxAngle;
              bounceAngle += incomingSign * glm::radians(5.0f);

              float speedMag = glm::length(speed);
              speed.x = -speedMag * sin(bounceAngle);
              speed.y = fabs(speedMag * cos(bounceAngle));

              ballLocation.y = blockTop + radius;
            }
          }
        }
      }
    }
  }
}

void Tema1::UpdateEnd(float deltaTimeSeconds) {
  if (lost) {
    text->Load("../assets/fonts/Hack-Bold.ttf", 100);
    text->RenderText("YOU DIED", 400, resolution.y / 2, 1);
  } else {
    text->Load("../assets/fonts/Hack-Bold.ttf", 100);
    text->RenderText("Victory Achieved", 200, resolution.y / 2, 1);
  }
}

void Tema1::FrameEnd() {}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Tema1::OnInputUpdate(float deltaTime, int mods) {
  if (inGame) {
    // cout << maxX - minX << '\n';
    if (window->KeyHold(GLFW_KEY_LEFT)) {
      if (shipUpdate.x - 300 * deltaTime - 30 - (-minX + maxX) * 60 / 2 >=
          -resolution.x / 2)
        shipUpdate -= 300 * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_RIGHT)) {
      if (shipUpdate.x + 300 * deltaTime + 30 + (-minX + maxX) * 60 / 2 <=
          resolution.x / 2)
        shipUpdate += 300 * deltaTime;
    }
  }
}
void Tema1::OnKeyPress(int key, int mods) {
  if (inGame == 1 && started == false && key == GLFW_KEY_SPACE) {
    start = 1;
  }
}

void Tema1::OnKeyRelease(int key, int mods) {
  // Add key release event
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
  // Add mouse move event
  if (inGame == 0)
    if (componentSelected != ComponentType::None) {
      translateX += deltaX;
      translateY += -1 * deltaY;
    }
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
  // Add mouse button press event

  if (inGame == 0) {
    if (button == GLFW_MOUSE_BUTTON_2) {
      // cout << "Mouse: X=" << mouseX << " Y=" << mouseY << "\n";
      mousePos.x = mouseX;
      mousePos.y = resolution.y - mouseY;

      if (mouseX <= componentsX) {
        for (int i = 0; i < shipComponents; ++i) {
          if (mouseY <= editorComponentsSize * (i + 1)) {
            componentSelected = ComponentTypes[i];
            break;
          }
        }
      } else {
        // Mesh *startButton = object::CreateRectangle(
        //     "startButton",
        //     glm::vec3(resolution.x - 200, resolution.y * 5 / 6, -1), 150,
        //     100, glm::vec3(0, 1, 0), true);
        int startX_min = resolution.x - 200;
        int startX_max = resolution.x - 50;
        int startY_max = resolution.y / 6;
        int startY_min = resolution.y / 6 - 100;

        if (mouseX >= startX_min && mouseX <= startX_max &&
            mouseY >= startY_min && mouseY <= startY_max && grid.ok) {
          inGame = true;
        }
      }
    }

    if (button == GLFW_MOUSE_BUTTON_3) {
      if (mouseX >= resolution.x / 6 + 25 &&
          mouseX <= resolution.x / 6 + 25 + gridSize.x * 60 &&
          mouseY <= resolution.y - 35 &&
          mouseY >= resolution.y - (35 + gridSize.y * 60)) {
        // cout << "GridSq X:" << mouseX << " Y:" << mouseY << "\n";
        for (int i = 0; i < gridSize.x; ++i) {
          for (int j = 0; j < gridSize.y; ++j) {
            int minX = resolution.x / 6 + 25 + i * 60;
            int maxX = resolution.x / 6 + 25 + (i + 1) * 60;
            int maxY = resolution.y - (35 + j * 60);
            int minY = resolution.y - (35 + (j + 1) * 60);

            if (mouseX >= minX && mouseX <= maxX && mouseY <= maxY &&
                mouseY >= minY) {
              if (ComponentType::None != grid.GetComponent(i, j)) {
                grid.RemoveComponent(i, j);
              }
            }
          }
        }
      }

      componentSelected = ComponentType::None;
      translateX = 0;
      translateY = 0;
    }
  }
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {

  if (inGame == 0) {
    if (button == GLFW_MOUSE_BUTTON_2) {
      if (grid.componentsNr && mouseX >= resolution.x / 6 + 25 &&
          mouseX <= resolution.x / 6 + 25 + gridSize.x * 60 &&
          mouseY <= resolution.y - 35 &&
          mouseY >= resolution.y - (35 + gridSize.y * 60)) {
        // cout << "GridSq X:" << mouseX << " Y:" << mouseY << "\n";
        for (int i = 0; i < gridSize.x; ++i) {
          for (int j = 0; j < gridSize.y; ++j) {
            int minX = resolution.x / 6 + 25 + i * 60;
            int maxX = resolution.x / 6 + 25 + (i + 1) * 60;
            int maxY = resolution.y - (35 + j * 60);
            int minY = resolution.y - (35 + (j + 1) * 60);

            if (mouseX >= minX && mouseX < maxX && mouseY < maxY &&
                mouseY >= minY) {
              grid.PlaceComponent(i, j, componentSelected);
              break;
            }
          }
        }
      }

      componentSelected = ComponentType::None;
      translateX = 0;
      translateY = 0;
    }
  }
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Tema1::OnWindowResize(int width, int height) {}
