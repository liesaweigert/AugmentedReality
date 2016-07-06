﻿#include <GL/glew.h>
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


GLfloat green[4] = { 0.0, 1.0, 0.0, 1.0 };
GLfloat red[4] = { 1.0, 0.0, 0.0, 1.0 };

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

void display_fallthrough(Eigen::Matrix4f marker_matrix){

    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    glColor4f(0.0, 0.0, 0.0, 1.0);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    //transform to marker
    glLoadTransposeMatrixf(marker_matrix.data());
    glTranslatef(0.0, 0.0, 0.06);
    drawCube(0.08);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}



int main(int argc, char* argv[])
{
	GLFWwindow* window;

    bool game_on = false;
    float start_game_time = 0.0;

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
    int correct = 0;
    int wrong = 0;

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
            start_game_time = glfwGetTime();
            form = rand() % 2;
        }

        display(window, img_bgr);

        //render correct and wrong answers
        String correct_string = "+" + std::to_string(correct);
        render_text(correct_string.c_str(), 20, 20, green, 48);

        String wrong_string = "-" + std::to_string(wrong);
        render_text(wrong_string.c_str(), 1200, 20, red, 48);

		/* Render here */
        if (game_on) {
            display_fallthrough(buttons[2].marker_matrix);
            display_form(form, buttons[2].marker_matrix, falling);
            falling += (glfwGetTime() - start_game_time) * 0.0002;
            if (falling >= 0.1){
                game_on = false;
                frames_botton_1_pressed = frames_button_0_pressed = 0;
                falling = -0.12;
                wrong++;
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
            falling = -0.12;
            if (frames_button_0_pressed > 10 && frames_botton_1_pressed > 10){
                wrong++;
            } else if (frames_button_0_pressed > 0 && form == 0){
                correct++;
            } else if (frames_botton_1_pressed > 0 && form == 1){
                correct++;
            } else wrong++;
            game_on = false;
            frames_botton_1_pressed = frames_button_0_pressed = 0;
        }

        if(glfwGetTime() >= 55 && glfwGetTime() <= 60){
            int restTime = 60 - (int)glfwGetTime();
            render_text(std::to_string(restTime).c_str(), 600, 300, red, 120);
        }

        if(glfwGetTime() > 60){
            game_on = false;
            frames_botton_1_pressed = frames_button_0_pressed = 0;
            falling = -0.12;
            String highscore = "Your new highscore is: " + std::to_string(correct);
            render_text(highscore.c_str(), 200, 300, green, 80);
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
