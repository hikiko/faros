#include <GL/glew.h>
#include <GL/freeglut.h>
#include "geom.h"
#include "sdr.h"

static unsigned int sdr_curve_top;

bool init_geom()
{
	if(!(sdr_curve_top = create_program_load("sdr/curve_top.v.glsl", "sdr/curve_top.f.glsl"))) {
		return false;
	}
	return true;
}

void destroy_geom()
{
	free_program(sdr_curve_top);
}

void faros()
{
	// kormos
	glPushMatrix();
	glScalef(1.1, 3, 1.1);
	glTranslatef(0, 0.5, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	glShadeModel(GL_FLAT);

	// base
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glTranslatef(0, -0.15, 0);
	glutSolidCylinder(2, 0.3, 16, 1);
	glPopMatrix();

	// middle cylinder
	glPushMatrix();
	glTranslatef(0, 3, 0);
	glRotatef(22.5, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCylinder(0.5, 1.0, 8, 1);
	glPopMatrix();

	// trim middle cylinder (mporntoura)
	glPushMatrix();
	glTranslatef(0, 3.9, 0);
	glRotatef(22.5, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCylinder(0.55, 0.02, 8, 1);
	glPopMatrix();

	// top smaller cylinder
	glPushMatrix();
	glTranslatef(0, 4, 0);
	glRotatef(22.5, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCylinder(0.28, 0.5, 8, 1);
	glPopMatrix();

	// top wire even smaller cylinder
	glPushMatrix();
	glTranslatef(0, 4.5, 0);
	glRotatef(22.5, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glutWireCylinder(0.18, 0.3, 9, 3);
	glPopMatrix();

	glShadeModel(GL_SMOOTH);

	// top troulos
	glPushMatrix();
	glTranslatef(0, 4.8, 0);
	glRotatef(22.5, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(0.18, 0.2, 9, 1);
	glPopMatrix();

	// tsamploukano
	glPushMatrix();
	glTranslatef(-0.28, 4, 0);
	glScalef(1, 13, 1);
	glutSolidSphere(0.1, 16, 16);
	glPopMatrix();

	//pyramid on top of kormos
	bind_program(sdr_curve_top);

	glPushMatrix();
	glTranslatef(0, 3, 0);
	glRotatef(45, 0, 1, 0);
	glRotatef(-90, 1, 0, 0);
	glScalef(1, 1, 0.45);
	glutSolidCylinder(1, 1, 4, 16);
	glPopMatrix();

	bind_program(0);
}


void ground()
{
	glPushMatrix();

	glTranslatef(0, -1.25, 0);
	glScalef(1, 0.1, 1);

	glutSolidSphere(10, 32, 32);

	glPopMatrix();
}

void xlogo()
{
}
