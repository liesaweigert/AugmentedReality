#include <GL/glew.h>
#define GLFW_INCLUDE_GLU

#include <iostream>
#include <GLFW/glfw3.h>
#include <opencv/cv.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include "opencv2/opencv.hpp"
#include "MarkerTracker.h"
#include "DrawPrimitives.h"
#include "RenderText.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader_utils.h"

//Mac
#include <GLUT/glut.h>

//Windows
//#include <GL/glut.h>

//download assimp first
//#include "AssimpModel.h"


using namespace cv;
using namespace std;

//camera settings
const int camera_width = 1280;
const int camera_height = 720;
const int virtual_camera_angle = 83;
unsigned char bkgnd[camera_width*camera_height * 3];

GLuint program;
GLint attribute_coord;
GLint uniform_tex;
GLint uniform_color;

struct point {
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
};

GLuint vbo;

FT_Library ft;
FT_Face face;


int init_resources() {
    /* Initialize the FreeType2 library */
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init freetype library\n");
        return 0;
    }

    /* Load a font */
    if (FT_New_Face(ft, "/Users/liesaweigert/ClionProjects/AR/assets/FreeSans.ttf", 0, &face)) {
        fprintf(stderr, "Could not open font!\n");
        return 0;
    }

    program = create_program("/Users/liesaweigert/ClionProjects/AR/src/text.v.glsl", "/Users/liesaweigert/ClionProjects/AR/src/text.f.glsl");
    if(program == 0)
        return 0;

    attribute_coord = get_attrib(program, "coord");
    uniform_tex = get_uniform(program, "tex");
    uniform_color = get_uniform(program, "color");

    if(attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1)
        return 0;

    // Create the vertex buffer object
    glGenBuffers(1, &vbo);

    return 1;
}

/* program & OpenGL initialization */
void initGL(int argc, char *argv[])
{
	// initialize the GL library
	// pixel storage/packing stuff
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // for glReadPixels​
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for glTexImage2D​
	glPixelZoom(1.0, -1.0);

	// enable and set colors
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0, 0, 0, 1.0);

	// enable and set depth parameters
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	// light parameters
	GLfloat light_pos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat light_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_dif[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	// enable lighting
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void initVideoStream(cv::VideoCapture &cap)
{

	if (cap.isOpened())
		cap.release();

	cap.open(0); // open the default camera
}

void display(GLFWwindow* window, const Mat &img_bgr)
{
	memcpy(bkgnd, img_bgr.data, sizeof(bkgnd));

	int width0, height0;
	glfwGetFramebufferSize(window, &width0, &height0);

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw background image
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, camera_width, 0.0, camera_height);

	glRasterPos2i(0, camera_height - 1);
    glDrawPixels(camera_width, camera_height, GL_BGR, GL_UNSIGNED_BYTE, bkgnd);

	glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

void display_form(int form, Eigen::Matrix4f marker_matrix, float falling){
    glMatrixMode(GL_MODELVIEW);

    glColor4f(1.0, 0.0, 0.0, 1.0);

    //transform to marker
    glLoadTransposeMatrixf(marker_matrix.data());
    glTranslatef(0.0, 0.0, falling);

    //move to marker position
    if(form == 0){
        drawSphere(0.04, 10, 100);
    } else {
        drawCube(0.04);
    }
}

/**
 * Render text using the currently loaded font and currently set font size.
 * Rendering starts at coordinates (x, y), z is always 0.
 * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
 */
void render_text(const char *text) {

    //rendering text
    float sx = 2.0 / 480.0;
    float sy = 2.0 / 640.0;

    float x = -1.0 + 8 * sx;
    float y = 1 - 50 * sy;

    glUseProgram(program);

    GLfloat black[4] = { 0.0, 0.0, 0.0, 1.0 };


    FT_Set_Pixel_Sizes(face, 0, 48);
    glUniform4fv(uniform_color, 1, black);

    const char *p;
    FT_GlyphSlot g = face->glyph;

    /* Create a texture that will be used to hold one "glyph" */
    GLuint tex;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uniform_tex, 0);

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Clamping to edges is important to prevent artifacts when scaling */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Linear filtering usually looks best for text */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Set up the VBO for our vertex data */
    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

    /* Loop through all characters */
    for (p = text; *p; p++) {
        /* Try to load and render the character */
        if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
            continue;

        /* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        /* Calculate the vertex and texture coordinates */
        float x2 = x + g->bitmap_left * sx;
        float y2 = -y - g->bitmap_top * sy;
        float w = g->bitmap.width * sx;
        float h = g->bitmap.rows * sy;

        point box[4] = {
                {x2, -y2, 0, 0},
                {x2 + w, -y2, 1, 0},
                {x2, -y2 - h, 0, 1},
                {x2 + w, -y2 - h, 1, 1},
        };

        /* Draw the character on the screen */
        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Advance the cursor to the start of the next character */
        x += (g->advance.x >> 6) * sx;
        y += (g->advance.y >> 6) * sy;
    }

    glDisableVertexAttribArray(attribute_coord);
    glDeleteTextures(1, &tex);
    glUseProgram(0);
}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

    bool game_on = false;

	/* Initialize the library */
	if (!glfwInit())
		return -1;


	// initialize the window system
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(camera_width, camera_height, "PokeDigi", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

    //create a random number (0 or 1 - sphere or cube)
    int form;
    int frames_button_0_pressed = 0;
    int frames_botton_1_pressed = 0;
	float falling = -0.12; //arbitrary value that looked ok

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);


	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    glewExperimental = GL_TRUE;
    glewInit();

    //init freetype resources
    init_resources();

	// initialize the GL library
	initGL(argc, argv);

	// setup OpenCV
	Mat img_bgr;
	VideoCapture cap;
	initVideoStream(cap);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(virtual_camera_angle, (double)window_width/(double)window_height, 0.01, 100);

    //set the two values for yes and no
    //sphere = 1228(4648) - cube =1c44(7236) - button for display = 02c2(706)
    Marker buttons[3];
    buttons[0].marker_code = 4648;
    buttons[1].marker_code = 7236;
    buttons[2].marker_code = 90;

    const GLubyte* opengl_version = glGetString(GL_VERSION);
    std::cout << "Using OpenGL Version: " << opengl_version << "\n";

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		cap >> img_bgr;

        flip(img_bgr, img_bgr, 1);

		if (img_bgr.empty()){
			std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
			initVideoStream(cap);
			cv::waitKey(1000); // Wait for one sec.
			continue;
		}

		/* Find the markers in the frame */
		MarkerTracker mt;
        mt.find(img_bgr, buttons, 3);

        //the game starts when both markers/buttons are visible
        if (!game_on && buttons[0].visible && buttons[1].visible){
            game_on = true;
            form = rand() % 2;
        }

        display(window, img_bgr);
        render_text("Loook at that fancy font!!");

		/* Render here */
        if (game_on) {
            display_form(form, buttons[2].marker_matrix, falling);
            falling += glfwGetTime() * 0.00005;
            if (falling >= 0.0){
                game_on = false;
                frames_botton_1_pressed = frames_button_0_pressed = 0;
                falling = -0.12;
            }
        }

        //see whether one marker is pressed
        if (game_on && !buttons[0].visible){
            frames_button_0_pressed++;
        }
        if(game_on && !buttons[1].visible){
            frames_botton_1_pressed++;
        }




        //after one marker has been pressed for 10 frames the result is
        //evaluated and the game starts over
        if (frames_button_0_pressed > 10 || frames_botton_1_pressed > 10){

            game_on = false;
            frames_botton_1_pressed = frames_button_0_pressed = 0;
            falling = -0.12;
        }

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	//freetype_terminate();
	glfwTerminate();

	return 0;
}
