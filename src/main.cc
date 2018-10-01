#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdlib.h>
#include <stdio.h>

#include "sdr.h"

#define CURVE_VS "sdr/curve_top.v.glsl"
#define CURVE_FS "sdr/curve_top.f.glsl"
#define BEAM_VS "sdr/beam.v.glsl"
#define BEAM_FS "sdr/beam.f.glsl"

#define BEAM_SHELLS 40
#define BEAM_RMIN 0.01
#define BEAM_RMAX 0.125
#define BEAM_ENERGY 0.02

static bool init();
static void cleanup();

static void faros();
static void light();
static void ground();
static void backdrop();

static void display();
static void idle();
static void reshape(int x, int y);
static void keyboard(unsigned char c, int x, int y);
static void mbutton(int bn, int state, int x, int y);
static void mmotion(int x, int y);

static float cam_theta = 45, cam_phi, cam_dist = 10;
static unsigned int sdr_curve_top, sdr_beam, sdr_sky;
static unsigned int start_time;
static float beam_rot_speed = 0.1;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glutCreateWindow("Faros");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mbutton);
	glutMotionFunc(mmotion);

	if(!init()) {
		return 1;
	}

	atexit(cleanup);
	glutMainLoop();

	return 0;
}

static bool init()
{
	glewInit();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

//	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);

	if(!(sdr_curve_top = create_program_load(CURVE_VS, CURVE_FS)))
		return false;

	if(!(sdr_beam = create_program_load(BEAM_VS, BEAM_FS)))
		return false;

	if(!(sdr_sky = create_program_load("sdr/sky.v.glsl", "sdr/sky.f.glsl"))) {
		return false;
	}

	start_time = glutGet(GLUT_ELAPSED_TIME);
	return true;
}

static void cleanup()
{
}

static void faros()
{
	glColor3f(0, 0, 0);

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

static void light()
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	glPushMatrix();

	glTranslatef(0, 4.65, 0.2);
	bind_program(sdr_beam);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for(int i=0; i<BEAM_SHELLS; i++) {
		float t = (float)i / (float)(BEAM_SHELLS - 1);
		float rad = BEAM_RMIN + (BEAM_RMAX - BEAM_RMIN) * t;
		float alpha = BEAM_ENERGY / t;

		glColor4f(0.8, 0.8, 0.2, alpha);

		glutSolidCylinder(rad, 6, 10, 1);
	}

	bind_program(0);

	glPopMatrix();

	glPopAttrib();
}

static void ground()
{
	glPushMatrix();

	glTranslatef(0, -1.25, 0);
	glScalef(1, 0.1, 1);

	glColor3f(0, 0, 0);
	glutSolidSphere(10, 32, 32);

	glPopMatrix();
}

static void backdrop()
{
	glFrontFace(GL_CW);
	bind_program(sdr_sky);
	glutSolidSphere(200, 16, 32);
	bind_program(0);
	glFrontFace(GL_CCW);
}

static void display()
{
	unsigned int tmsec = glutGet(GLUT_ELAPSED_TIME) - start_time;
	float tsec = (float)tmsec / 1000.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	backdrop();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, -2, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	ground();
	faros();

	glPushMatrix();

	float beam_angle = tsec * beam_rot_speed * 360;

	glRotatef(beam_angle, 0, 1, 0);
	light();

	glPopMatrix();

	glutSwapBuffers();
}

static void idle()
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(50, (float)x / (float)y, 0.5, 500);
}

static void keyboard(unsigned char c, int x, int y)
{
	switch(c) {
	case 27:
		exit(0);
	default:
		break;
	}
}

static int prev_x, prev_y;
static bool bst[8];
static void mbutton(int bn, int state, int x, int y)
{
	int button = bn - GLUT_LEFT_BUTTON;
	bst[button] = state == GLUT_DOWN;

	prev_x = x;
	prev_y = y;
}

static void mmotion(int x, int y)
{
	int dx = x - prev_x;
	int dy = y - prev_y;

	prev_x = x;
	prev_y = y;

	if (dx == 0 && dy == 0)
		return;

	if (bst[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if (cam_phi < -90)
			cam_phi = -90;

		if (cam_phi > 90)
			cam_phi = 90;
	}

	if (bst[2]) {
		cam_dist += dy * 0.1;

		if (cam_dist < 0)
			cam_dist = 0;
	}
}
