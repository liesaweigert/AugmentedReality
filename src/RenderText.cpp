//
// Created by Liesa  on 20/06/16.
//

#include "RenderText.h"

std::map<GLchar, Character> characters;

GLuint VAO, VBO;

GLuint program;
GLint attribute_coord;
GLint uniform_tex;
GLint uniform_color;

GLfloat black[4] = { 0, 0, 0, 1 };

const GLuint WIDTH = 800, HEIGHT = 600;

void render_text(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {

    glMatrixMode(GL_MODELVIEW);

    glUseProgram(program);
    glUniform4fv(uniform_color, 1, black);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArrayAPPLE(VAO);

    // Iterate through all characters of the message
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;


        // Update VBO for each character
        point box[4] = {
                {xpos, -ypos, 0, 0},
                {xpos + w, -ypos, 1, 0},
                {xpos, -ypos - h, 0, 1},
                {xpos + w, -ypos - h, 1, 1},
        };

        /* Draw the character on the screen */
        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



        GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArrayAPPLE(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}
void init_freetype(){
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    FT_Library ft;

    if(FT_Init_FreeType(&ft)) {
        std::cout << "ERROR: Could not init freetype library\n";
    }

    FT_Face face;
    if (FT_New_Face(ft, "/Users/liesaweigert/ClionProjects/AR/assets/FreeSans.ttf", 0, &face)){
        std::cout << "ERROR: Could not open font\n";
    }

    program = create_program("/Users/liesaweigert/ClionProjects/AR/src/RenderText.vert","/Users/liesaweigert/ClionProjects/AR/src/RenderText.frag");
    if (program == 0){
        std::cout << "ERROR: Could not load shaders";
    }

    attribute_coord = get_attrib(program, "coord");
    uniform_tex = get_uniform(program, "tex");
    uniform_color = get_uniform(program, "color");

    if(attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1) {
        std::cout << "ERROR: Could not load attributes";
    }

    glGenBuffers(1, &VBO);

    //height: 48, width: automatic
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Load all first 128 ascii characters
    for (GLubyte i = 0; i < 128; i++){
        //the current glyph into the face
        if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cout << "ERROR: Could not load glyph";
            continue;
        }

        //generating a texture for the glyph
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        //set texture options

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //and now we store the character for later usage (which we wanted all along)
        Character character = {
                texture,
                ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)
        };

        characters.insert(std::pair<GLchar, Character>(i, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for texture quads
    glGenVertexArraysAPPLE(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArrayAPPLE(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArrayAPPLE(0);
}



