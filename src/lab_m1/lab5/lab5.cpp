#include "lab_m1/lab5/lab5.h"

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

Lab5::Lab5() {}

Lab5::~Lab5() {}

void Lab5::Init() {
  renderCameraTarget = false;
  object = 0;

  bonus = false;

  fov = RADIANS(60);
  type = false;

  camera = new implemented::Camera();
  camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

  {
    Mesh *mesh = new Mesh("box");
    mesh->LoadMesh(
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
        "box.obj");
    meshes[mesh->GetMeshID()] = mesh;
  }

  {
    Mesh *mesh = new Mesh("sphere");
    mesh->LoadMesh(
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
        "sphere.obj");
    meshes[mesh->GetMeshID()] = mesh;
  }

  // TODO(student): After you implement the changing of the projection
  // parameters, remove hardcodings of these parameters
  left = -5.0f;
  right = 5.0f;
  bottom = -5.0f;
  top = 5.0f;
  znear = 0.1f;
  zfar = 100.0f;
}

void Lab5::FrameStart() {
  // Clears the color buffer (using the previously set color) and depth buffer
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::ivec2 resolution = window->GetResolution();
  // Sets the screen area where to draw
  glViewport(0, 0, resolution.x, resolution.y);
}

void Lab5::Update(float deltaTimeSeconds) {

  if (type == false) {
    projectionMatrix =
        glm::perspective(fov, window->props.aspectRatio, 0.01f, 200.0f);
  } else {
    projectionMatrix = glm::ortho(left, right, bottom, top, znear, zfar);
  }

  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, RADIANS(45.0f), glm::vec3(0, 1, 0));

    RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);
  }

  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2, 0.5f, 0));
    modelMatrix = glm::rotate(modelMatrix, RADIANS(60.0f), glm::vec3(1, 0, 0));
    RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);
  }

  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2, 0.5f, 0));
    RenderMesh(meshes["box"], shaders["Simple"], modelMatrix);
  }

  glm::vec3 cameraTargetPos = camera->GetTargetPosition();
  glm::vec3 objPos = glm::vec3(0, 0, 0);
  if (object == 0)
    objPos = glm::vec3(0, 1, 0);
  if (object == 1)
    objPos = glm::vec3(2, 0.5f, 0);
  if (object == 2)
    objPos = glm::vec3(-2, 0.5f, 0);

  if (bonus) {
    glm::vec3 desiredDir = glm::normalize(objPos - camera->position);
    glm::vec3 currentDir = glm::normalize(camera->forward);

    glm::vec3 flatCurrent =
        glm::normalize(glm::vec3(currentDir.x, 0, currentDir.z));
    glm::vec3 flatDesired =
        glm::normalize(glm::vec3(desiredDir.x, 0, desiredDir.z));

    float dotYaw = glm::clamp(glm::dot(flatCurrent, flatDesired), -1.0f, 1.0f);
    float angleYaw = acos(dotYaw);

    float signYaw;
    if (glm::cross(flatCurrent, flatDesired).y >= 0) {
      signYaw = 1.0f;
    } else {
      signYaw = -1.0f;
    }

    float desiredPitch = asin(desiredDir.y);
    float currentPitch = asin(currentDir.y);
    float deltaPitch = desiredPitch - currentPitch;

    deltaPitch = glm::clamp(deltaPitch, -0.05f, 0.05f);

    float rotationSpeed = 1.2f * deltaTimeSeconds;

    if (angleYaw > 0.001f)
      camera->RotateFirstPerson_OY(signYaw * glm::min(rotationSpeed, angleYaw));

    if (fabs(deltaPitch) > 0.001f)
      camera->RotateFirstPerson_OX(deltaPitch * 0.5f);
  }

  // TODO(student): Draw more objects with different model matrices.
  // Attention! The `RenderMesh()` function overrides the usual
  // `RenderMesh()` that we've been using up until now. This new
  // function uses the view matrix from the camera that you just
  // implemented, and the local projection matrix.

  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(2, 2.5f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
  }

  {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-5, 2, 0));
    RenderMesh(meshes["sphere"], shaders["Simple"], modelMatrix);
  }

  // Render the camera target. This is useful for understanding where
  // the rotation point is, when moving in third-person camera mode.
  if (renderCameraTarget) {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    RenderMesh(meshes["sphere"], shaders["VertexNormal"], modelMatrix);
  }
}

void Lab5::FrameEnd() {
  DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Lab5::RenderMesh(Mesh *mesh, Shader *shader,
                      const glm::mat4 &modelMatrix) {
  if (!mesh || !shader || !shader->program)
    return;

  // Render an object using the specified shader and the specified position
  shader->Use();
  glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                     glm::value_ptr(camera->GetViewMatrix()));
  glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                     glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                     glm::value_ptr(modelMatrix));

  mesh->Render();
}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Lab5::OnInputUpdate(float deltaTime, int mods) {
  // move the camera only if MOUSE_RIGHT button is pressed
  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    float cameraSpeed = 2.0f;

    if (window->KeyHold(GLFW_KEY_W)) {
      // TODO(student): Translate the camera forward
      camera->TranslateForward(deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_A)) {
      // TODO(student): Translate the camera to the left
      camera->TranslateRight(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_S)) {
      // TODO(student): Translate the camera backward
      camera->TranslateForward(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_D)) {
      // TODO(student): Translate the camera to the right
      camera->TranslateRight(deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_Q)) {
      // TODO(student): Translate the camera downward
      camera->TranslateUpward(-deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_E)) {
      // TODO(student): Translate the camera upward
      camera->TranslateUpward(deltaTime);
    }
  }

  // TODO(student): Change projection parameters. Declare any extra
  // variables you might need in the class header. Inspect this file
  // for any hardcoded projection arguments (can you find any?) and
  // replace them with those extra variables.
  if (window->KeyHold(GLFW_KEY_1) && type == true) {
    // TODO(student): Translate the camera upward
    left -= deltaTime;
    right += deltaTime;

    bottom -= deltaTime;
    top += deltaTime;
  }

  if (window->KeyHold(GLFW_KEY_2) && type == true) {
    // TODO(student): Translate the camera upward
    left += deltaTime;
    right -= deltaTime;

    bottom += deltaTime;
    top -= deltaTime;
  }

  if (window->KeyHold(GLFW_KEY_3) && type == false) {
    // TODO(student): Translate the camera upward
    fov += deltaTime;
  }

  if (window->KeyHold(GLFW_KEY_4) && type == false) {
    // TODO(student): Translate the camera upward
    fov -= deltaTime;
  }
}

void Lab5::OnKeyPress(int key, int mods) {
  // Add key press event
  if (key == GLFW_KEY_T) {
    renderCameraTarget = !renderCameraTarget;
  }
  if (key == GLFW_KEY_B) {
    bonus = !bonus;
    cout << bonus << '\n';
  }
  if (key == GLFW_KEY_O) {
    type = true;
  }
  if (key == GLFW_KEY_P) {
    type = false;
  }

  if (key == GLFW_KEY_8) {
    object = 0;
  }
  if (key == GLFW_KEY_9) {
    object = 1;
  }
  if (key == GLFW_KEY_0) {
    object = 2;
  }
  // TODO(student): Switch projections
}

void Lab5::OnKeyRelease(int key, int mods) {
  // Add key release event
}

void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
  // Add mouse move event

  if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
    float sensivityOX = 0.001f;
    float sensivityOY = 0.001f;

    if (window->GetSpecialKeyState() == 0) {
      renderCameraTarget = false;
      // TODO(student): Rotate the camera in first-person mode around
      // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
      // variables for setting up the rotation speed.
      camera->RotateFirstPerson_OX(-deltaY * sensivityOX);
      camera->RotateFirstPerson_OY(-deltaX * sensivityOY);
    }

    if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
      renderCameraTarget = true;
      // TODO(student): Rotate the camera in third-person mode around
      // OX and OY using `deltaX` and `deltaY`. Use the sensitivity
      // variables for setting up the rotation speed.
      camera->RotateThirdPerson_OX(-deltaY * sensivityOX);
      camera->RotateThirdPerson_OY(-deltaX * sensivityOY);
    }
  }
}

void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {
  // Add mouse button press event
}

void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
  // Add mouse button release event
}

void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Lab5::OnWindowResize(int width, int height) {}
