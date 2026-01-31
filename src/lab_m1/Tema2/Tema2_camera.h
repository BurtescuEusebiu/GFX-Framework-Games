#pragma once

#include "utils/glm_utils.h"
#include "utils/math_utils.h"

namespace implemented {
class Tema2Camera {
public:
  Tema2Camera() {
    position = glm::vec3(0, 2, 5);
    forward = glm::vec3(0, 0, -1);
    up = glm::vec3(0, 1, 0);
    right = glm::vec3(1, 0, 0);
    distanceToTarget = 2;
    cameraSpeed = 10.0f;
  }

  Tema2Camera(const glm::vec3 &position, const glm::vec3 &center,
              const glm::vec3 &up) {
    Set(position, center, up);
  }

  ~Tema2Camera() {}

  void Set(const glm::vec3 &position, const glm::vec3 &center,
           const glm::vec3 &up) {
    this->position = position;
    forward = glm::normalize(center - position);
    right = glm::cross(forward, up);
    this->up = glm::cross(right, forward);
  }

  void MoveForward(float distance) {
    glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
    position += dir * distance * cameraSpeed;
  }

  void TranslateForward(float distance) {
    glm::vec3 dir = glm::normalize(forward);
    position += dir * distance * cameraSpeed;
  }

  void TranslateUpward(float distance) {
    glm::vec3 dir = glm::normalize(up);
    position += dir * distance * cameraSpeed;
  }

  void TranslateRight(float distance) {
    glm::vec3 dir = glm::normalize(glm::vec3(right.x, 0, right.z));
    position += dir * distance * cameraSpeed;
  }

  void RotateFirstPerson_OX(float angle) {
    glm::vec4 newVector =
        glm::rotate(glm::mat4(1.0f), angle, right) * glm::vec4(forward, 1);
    forward = glm::normalize(glm::vec3(newVector));
    up = glm::normalize(glm::cross(right, forward));
  }

  void RotateFirstPerson_OY(float angle) {
    glm::vec4 newVector =
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
        glm::vec4(forward, 1);
    forward = glm::normalize(glm::vec3(newVector));
    newVector = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) *
                glm::vec4(right, 1);
    right = glm::normalize(glm::vec3(newVector));
    up = glm::normalize(glm::cross(right, forward));
  }

  void RotateFirstPerson_OZ(float angle) {

    glm::vec4 newVector =
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) *
        glm::vec4(right, 1);
    right = glm::normalize(glm::vec3(newVector));
    up = glm::cross(right, forward);
  }

  void RotateThirdPerson_OX(float angle) {
    TranslateForward(distanceToTarget);
    RotateFirstPerson_OX(angle);
    TranslateForward(-distanceToTarget);
  }

  void RotateThirdPerson_OY(float angle) {
    TranslateForward(distanceToTarget);
    RotateFirstPerson_OY(angle);
    TranslateForward(-distanceToTarget);
  }

  void RotateThirdPerson_OZ(float angle) {
    TranslateForward(distanceToTarget);
    RotateFirstPerson_OZ(angle);
    TranslateForward(-distanceToTarget);
  }

  glm::mat4 GetViewMatrix() {
    return glm::lookAt(position, position + forward, up);
  }

  glm::mat4 GetProjectionMatrix(float fov, float aspectRatio, float nearPlane,
                                float farPlane) {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane,
                            farPlane);
  }

  glm::vec3 GetTargetPosition() {
    return position + forward * distanceToTarget;
  }

public:
  float distanceToTarget;
  float cameraSpeed;
  glm::vec3 position;
  glm::vec3 forward;
  glm::vec3 right;
  glm::vec3 up;
};
} // namespace implemented
