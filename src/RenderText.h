//
// Created by Liesa  on 20/06/16.
//

#ifndef AR_RENDERTEXT_H
#define AR_RENDERTEXT_H

#include <GL/glew.h>

int init_resources();

void render_text(const char *text, int x, int y, GLfloat* color, int size);

#endif //AR_RENDERTEXT_H
