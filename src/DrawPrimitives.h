//
// Created by Liesa  on 03/06/16.
//

#ifndef OPENGLSTARTER_DRAWPRIMITIVES_H
#define OPENGLSTARTER_DRAWPRIMITIVES_H

#define GLFW_INCLUDE_GLU
#include <math.h>
#include <GLFW/glfw3.h>

void drawSphere(double r, int lats, int longs);
void drawCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
void drawCube(double size);

#endif //OPENGLSTARTER_DRAWPRIMITIVES_H
