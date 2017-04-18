#ifndef CONTROLS_HPP
#define CONTROLS_HPP
#include "common.h"

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

void computeViewProjectionMatrices(GLFWwindow* window);


#endif
