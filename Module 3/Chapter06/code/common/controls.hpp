#ifndef CONTROLS_HPP
#define CONTROLS_HPP
#include "common.h"

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

void computeViewProjectionMatrices(GLFWwindow* window);
void computeStereoViewProjectionMatrices(GLFWwindow* window, float IPD, float depthZ, bool left_eye);

#endif
