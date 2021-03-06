//
// Created by Liesa  on 03/06/16.
//

#include "DrawPrimitives.h"


/* PI */
#ifndef M_PI
 #define M_PI 3.1415926535897932384626433832795
#endif


void drawSphere(double r, int lats, int longs) {
	int i, j;
	for(i = 0; i <= lats; i++) {
		double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
		double z0  = r * sin(lat0);
		double zr0 = r *  cos(lat0);

		double lat1 = M_PI * (-0.5 + (double) i / lats);
		double z1  = r * sin(lat1);
		double zr1 = r * cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for(j = 0; j <= longs; j++) {
			double lng = 2 * M_PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
}


void drawCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);

	gluCylinder(quadric, base, (GLdouble)0.0, height, slices, stacks);
	const GLdouble innerradius = 0.0;
	gluDisk(quadric, innerradius, base, slices, stacks);

	gluDeleteQuadric(quadric);
}


void drawCube(double size){
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
		glNormal3f(0.0, 0.0, -n);
	}
	glEnd();
}