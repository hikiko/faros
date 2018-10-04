#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "sdr.h"
#include "geom.h"
#include "seq.h"

#define BEAM_SHELLS 40
#define BEAM_RMIN 0.01
#define BEAM_RMAX 0.125
#define BEAM_ENERGY 0.02
#define BEAM_LEN 16.0
#define BEAM_DEF_SPEED	0.1

static bool init();
static void cleanup();

static void display();
static void light();
static void backdrop();

static void idle();
static void reshape(int x, int y);
static void keyboard(unsigned char c, int x, int y);
static void mbutton(int bn, int state, int x, int y);
static void mmotion(int x, int y);

static float cam_theta = 45, cam_phi, cam_dist = 10;
static unsigned int sdr_beam, sdr_sky;
static long start_time;
static long anim_stop_time;
static long tmsec, prev_tmsec;

static const float sil_color[] = {0.05, 0.02, 0.1, 1.0};
static const float beam_color[] = {0.5, 0.4, 0.2, 1.0};

static float beam_angle, beam_speed;

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

	if(!init_geom()) {
		return false;
	}
	if(!(sdr_beam = create_program_load("sdr/beam.v.glsl", "sdr/beam.f.glsl")))
		return false;

	if(!(sdr_sky = create_program_load("sdr/sky.v.glsl", "sdr/sky.f.glsl"))) {
		return false;
	}

	if(!init_seq()) {
		return false;
	}
	add_seq_track("beam-speed", INTERP_SIGMOID, EXTRAP_CLAMP, BEAM_DEF_SPEED);
	load_seq("seq");

	start_time = glutGet(GLUT_ELAPSED_TIME);
	prev_tmsec = start_time;
	return true;
}

static void cleanup()
{
	destroy_seq();
	destroy_geom();
	free_program(sdr_beam);
	free_program(sdr_sky);
}

static void display()
{
	tmsec = (long)glutGet(GLUT_ELAPSED_TIME) - start_time;
	float dt = (tmsec - prev_tmsec) / 1000.0f;
	prev_tmsec = tmsec;

	if(anim_stop_time) dt = 0.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	backdrop();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, -2, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	glColor3fv(sil_color);
	ground();
	faros();

	glPushMatrix();

	beam_speed = get_seq_value("beam-speed", tmsec);
	beam_angle += beam_speed * 360.0f * dt;
	glRotatef(beam_angle, 0, 1, 0);
	light();

	glPopMatrix();

	glutSwapBuffers();
}

static void light()
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	glPushMatrix();

	glTranslatef(0, 4.65, 0.2);
	bind_program(sdr_beam);
	set_uniform_float(sdr_beam, "beam_len", BEAM_LEN);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for(int i=0; i<BEAM_SHELLS; i++) {
		float t = (float)i / (float)(BEAM_SHELLS - 1);
		float rad = BEAM_RMIN + (BEAM_RMAX - BEAM_RMIN) * t;
		float alpha = BEAM_ENERGY / (t * t);

		glColor4f(beam_color[0], beam_color[1], beam_color[2], alpha);

		glutSolidCylinder(rad, BEAM_LEN, 12, 1);
	}

	bind_program(0);

	glPopMatrix();

	glPopAttrib();
}

static void backdrop()
{
	glFrontFace(GL_CW);
	bind_program(sdr_sky);
	glutSolidSphere(200, 16, 32);
	bind_program(0);
	glFrontFace(GL_CCW);
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

#define ANIM_DELTA	0.5

static void keyboard(unsigned char c, int x, int y)
{
	int idx;
	static float orig_beam_speed;

	switch(c) {
	case 27:
		exit(0);

	case '\b':
		start_time = glutGet(GLUT_ELAPSED_TIME);
		prev_tmsec = 0;
		anim_stop_time = 0;
		beam_angle = 0;
		break;

	case ' ':
		if(anim_stop_time > 0) {
			long msec = glutGet(GLUT_ELAPSED_TIME);
			start_time += msec - anim_stop_time;
			prev_tmsec = msec - start_time;
			anim_stop_time = 0;
		} else {
			anim_stop_time = glutGet(GLUT_ELAPSED_TIME);
		}
		break;

	case '=':
		beam_speed = get_seq_value("beam-speed", tmsec);
		clear_seq_track("beam-speed");
		set_seq_value("beam-speed", tmsec, beam_speed + ANIM_DELTA);
		break;

	case '-':
		beam_speed = get_seq_value("beam-speed", tmsec) - ANIM_DELTA;
		if(beam_speed < 0)
			beam_speed = 0;
		clear_seq_track("beam-speed");
		set_seq_value("beam-speed", tmsec, beam_speed);
		break;

	case '\r':
	case '\n':
		idx = find_seq_track("beam-speed");
		assert(idx >= 0);
		if(get_seq_value(idx, tmsec) > 0.0) {
			clear_seq_track(idx);
			set_seq_value(idx, tmsec, beam_speed);
			set_seq_value(idx, tmsec + 3000, 0);
			orig_beam_speed = beam_speed;
		} else {
			clear_seq_track(idx);
			set_seq_value(idx, tmsec, 0);
			set_seq_value(idx, tmsec + 3000, orig_beam_speed);
		}
		break;

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
