//
// Created by Liesa  on 20/06/16.
//

#ifndef AR_RENDERTEXT_H
#define AR_RENDERTEXT_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>
#include <math.h>
#include <GLUT/glut.h>
#include <glm/glm.hpp>
#include "Shader.h"

//install with 'brew install freetype' or whatever windows does
#include <ft2build.h>
#include FT_FREETYPE_H

//Properties
#include "Shader.h"

using namespace glm;

struct Character{
    GLuint texture_id;
    ivec2 size;
    ivec2 bearing;
    GLuint advance;
};

void init_freetype();

void render_text(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

#endif //AR_RENDERTEXT_H
