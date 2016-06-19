#define GLFW_INCLUDE_GLU

#include <iostream>
#include <GLFW/glfw3.h>
#include <opencv/cv.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include "opencv2/opencv.hpp"
#include "MarkerTracker.h"

using namespace cv;
using namespace std;



//camera settings
const int camera_width = 1280;
const int camera_height = 720;
const int virtual_camera_angle = 83;
unsigned char bkgnd[camera_width*camera_height * 3];

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
	GLfloat light_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat light_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_dif[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	// enable lighting
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
}

void initVideoStream(cv::VideoCapture &cap)
{

	if (cap.isOpened())
		cap.release();

	cap.open(0); // open the default camera
}

void display(GLFWwindow* window, const Mat &img_bgr, Eigen::Matrix4f marker_matrix)
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

	glMatrixMode(GL_MODELVIEW);

    //move to marker position
    glLoadTransposeMatrixf(marker_matrix.data());


    //render square
    double size = 0.04;
    double size_h = 0.02;
    glColor3f(1.0, 0.0, 1.0);

    glBegin(GL_QUADS);
        glVertex3f(+size/2.0, +size/2.0, 0);
        glVertex3f(-size/2.0, +size/2.0, 0);
        glVertex3f(-size/2.0, +size/2.0, -size);
        glVertex3f(+size/2.0, +size/2.0, -size);
        glNormal3f(0.0, 1.0, 0.0);

        glVertex3f(-size/2.0, +size/2.0, 0);
        glVertex3f(-size/2.0, -size/2.0, 0);
        glVertex3f(-size/2.0, -size/2.0, -size);
        glVertex3f(-size/2.0, +size/2.0, -size);
        glNormal3f(-1.0, 0.0, 0.0);

        glVertex3f(+size/2.0, -size/2.0, 0);
        glVertex3f(-size/2.0, -size/2.0, 0);
        glVertex3f(-size/2.0, -size/2.0, -size);
        glVertex3f(+size/2.0, -size/2.0, -size);
        glNormal3f(0.0, -1.0, 0.0);

        glVertex3f(+size/2.0, +size/2.0, 0);
        glVertex3f(+size/2.0, -size/2.0, 0);
        glVertex3f(+size/2.0, -size/2.0, -size);
        glVertex3f(+size/2.0, +size/2.0, -size);
        glNormal3f(1.0, 0.0, 0.0);

        //upper and lower quads
        for (double j = 0; j >= -size; j -= size){
            int n = (j == 0) ? -1 : 1;
            glVertex3f(+size/2.0, +size/2.0, j);
            glVertex3f(-size/2.0, +size/2.0, j);
            glVertex3f(-size/2.0, -size/2.0, j);
            glVertex3f(+size/2.0, -size/2.0, j);
            glNormal3f(0.0, 0.0, n);
        }
    glEnd();

    //render snowman
    /*glColor4f(1.0, 1.0, 1.0, 1.0);
    glPushMatrix();
        drawSphere(0.04, 10, 100);
        glPushMatrix();
            glTranslatef(0.0, 0.0, 0.04);
            drawSphere(0.02, 10, 10);
    glPopMatrix();*/

}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// initialize the window system
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(camera_width, camera_height, "Combine", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

	// initialize the GL library
	initGL(argc, argv);

	// setup OpenCV
	Mat img_bgr;
	VideoCapture cap;
	initVideoStream(cap);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(virtual_camera_angle, (double)window_width/(double)window_height, 0.01, 100);


	Eigen::Matrix4f marker_matrix;

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
        marker_matrix = mt.find(img_bgr);

		/* Render here */
        display(window, img_bgr, marker_matrix);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
