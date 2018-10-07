#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "geom.h"
#include "sdr.h"

static unsigned int sdr_curve_top;
static unsigned int tex_xcircle;

static const unsigned char tex_xcircle_pixels[] = {
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,
	0, 64, 255, 255, 255, 255, 64, 0,

	0, 64, 255, 255, 255, 255, 64, 0,
	0, 0, 255, 255, 255, 255, 0, 0,
	0, 0, 128, 255, 255, 128, 0, 0,
	0, 0, 0, 64, 64, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


bool init_geom()
{
	if(!(sdr_curve_top = create_program_load("sdr/curve_top.v.glsl", "sdr/curve_top.f.glsl"))) {
		return false;
	}

	glGenTextures(1, &tex_xcircle);
	glBindTexture(GL_TEXTURE_2D, tex_xcircle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 8, 16, 0,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, tex_xcircle_pixels);
	return true;
}

void destroy_geom()
{
	glDeleteTextures(1, &tex_xcircle);
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

void xlogo(float sz, const float *col_ink, const float *col_paper, float alpha, float xcircle)
{
	static const float xlogo_varr[] = {
		-0.500, 0.407, -0.113, -0.109, 0.059, -0.006, -0.251, 0.407,
		-0.113, -0.109, -0.499, -0.593, -0.410, -0.593, 0.059, -0.006,
		-0.058, -0.182, 0.251, -0.593, 0.500, -0.593, 0.114, -0.079,
		-0.058, -0.182, 0.114, -0.079, 0.500, 0.407, 0.411, 0.407
	};

	/* billboarding */
	float mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	mv[0] = mv[5] = mv[10] = sz;
	mv[1] = mv[2] = mv[4] = mv[6] = mv[8] = mv[9] = 0.0f;

	glPushMatrix();
	glLoadMatrixf(mv);
	glTranslatef(0, 0.15, 0);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor4f(col_ink[0], col_ink[1], col_ink[2], alpha);
	const float *vptr = xlogo_varr;
	for(int i=0; i<(int)(sizeof xlogo_varr / sizeof *xlogo_varr) / 2; i++) {
		glVertex2fv(vptr);
		vptr += 2;
	}
	glEnd();
	glTranslatef(0, -0.15, 0);
	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_xcircle);

	glDisable(GL_CULL_FACE);
	glEnable(GL_ALPHA_TEST);
	float aref = 1.0f - xcircle;
	glAlphaFunc(GL_GREATER, aref > 0.0f ? aref : 0.0f);

	glScalef(1.4, 1, 1);

#define XLOGO_CIRCLE_SEG	64
	// circle thingy
	glBegin(GL_QUAD_STRIP);
	for(int i=0; i<XLOGO_CIRCLE_SEG; i++) {
		float t = (float)i / (float)(XLOGO_CIRCLE_SEG - 1);
		float tcol = fmod(t + 0.075f, 1.0f);

		float theta = t * M_PI * 2.0f;
		float rad = 0.4f;
		float width = 0.05f * tcol;
		float z = -cos(theta) * 0.1;

		glColor4f(col_paper[0], col_paper[1], col_paper[2], tcol);
		glTexCoord2f(0, 1.0f - tcol);
		glVertex3f(sin(theta) * (rad + width), cos(theta) * (rad + width), z);
		glTexCoord2f(1, 1.0f - tcol);
		glVertex3f(sin(theta) * (rad - width), cos(theta) * (rad - width), z);
	}
	glEnd();

	glPopAttrib();
	glPopMatrix();
}
