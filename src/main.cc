#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "sdr.h"
#include "geom.h"
#include "seq.h"
#include "ui.h"

#define BEAM_SHELLS 40
#define BEAM_RMIN 0.01
#define BEAM_RMAX 0.125
#define BEAM_ENERGY 0.01
#define BEAM_LEN 16.0
#define BEAM_DEF_SPEED	0.1
#define BEAM_HEIGHT		4.65

#define XLOGO_RAD	0.5

#define LERP(a, b, t)	((a) + ((b) - (a)) * (t))

struct Camera {
	float x, y, z;
	float theta, phi;
	float dist;
};

static bool init();
static void cleanup();

static void display();
static void light();
static void paper_light();
static void backdrop(float alpha);
static void help();

static void idle();
static void reshape(int x, int y);
static void keyboard(unsigned char c, int x, int y);
static void keyb_special(int key, int x, int y);
static void mbutton(int bn, int state, int x, int y);
static void mmotion(int x, int y);

int win_width, win_height;
static bool freecam = true;

static Camera cam = {0, 0, 0, 0, 0, 10};
static unsigned int sdr_beam, sdr_sky;
static long start_time;
static long anim_stop_time;
long tmsec, prev_tmsec, anim_time;
bool anim_stopped;

static const float sil_color[] = {0.05, 0.02, 0.1, 1.0};
static const float ink_color[] = {0.05, 0.15, 0.3, 1.0};
static const float beam_color[] = {0.5, 0.4, 0.2, 1.0};
static const float paper_color[] = {0.9, 0.8, 0.8, 1.0};

static float beam_angle, beam_speed;
static float beam_len;
static float xlogo_alpha, xcircle;
static float paper;

static bool show_help, show_ui = true;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glutCreateWindow("Faros (press F1 for controls help)");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyb_special);
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
	add_seq_track("beam-len", INTERP_SIGMOID, EXTRAP_CLAMP, BEAM_LEN);
	add_seq_track("cam-dist", INTERP_SIGMOID, EXTRAP_CLAMP, 10);
	add_seq_track("cam-phi", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("cam-theta", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("cam-x", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("cam-y", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("cam-z", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("xlogo", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	add_seq_track("xcircle", INTERP_SIGMOID, EXTRAP_CLAMP, 1);
	add_seq_track("paper", INTERP_SIGMOID, EXTRAP_CLAMP, 0);
	load_seq("seq");

	freecam = seq_track_empty("cam-theta");

	glClearColor(paper_color[0], paper_color[1], paper_color[2], 1);

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

	anim_time = tmsec;
	if(anim_stopped) {
		dt = 0.0f;
		anim_time = anim_stop_time - start_time;
	}

	paper = get_seq_value("paper", anim_time);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	backdrop(1.0f - paper);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!freecam) {
		cam.dist = get_seq_value("cam-dist", anim_time);
		cam.phi = get_seq_value("cam-phi", anim_time);
		cam.theta = get_seq_value("cam-theta", anim_time);
		cam.x = get_seq_value("cam-x", anim_time);
		cam.y = get_seq_value("cam-y", anim_time);
		cam.z = get_seq_value("cam-z", anim_time);
	}

	glTranslatef(0, -2, -cam.dist);
	glRotatef(cam.phi, 1, 0, 0);
	glRotatef(cam.theta, 0, 1, 0);
	glTranslatef(-cam.x, -cam.y, -cam.z);

	float color[3];
	color[0] = LERP(sil_color[0], ink_color[0], paper);
	color[1] = LERP(sil_color[1], ink_color[1], paper);
	color[2] = LERP(sil_color[2], ink_color[2], paper);

	glColor3fv(color);
	ground();
	faros();

	beam_len = get_seq_value("beam-len", anim_time);
	beam_speed = get_seq_value("beam-speed", anim_time);
	beam_angle += beam_speed * 360.0f * dt;

	xlogo_alpha = get_seq_value("xlogo", anim_time);
	xcircle = get_seq_value("xcircle", anim_time);

	if(xlogo_alpha > 0.0) {
		glPushMatrix();
		float beam_angle_rad = beam_angle / 180.0 * M_PI;
		float xlogo_dist = beam_len;
		float xlogo_pos[3] = {sin(beam_angle_rad), 0, cos(beam_angle_rad)};
		glTranslatef(xlogo_pos[0] * xlogo_dist, xlogo_pos[1] * xlogo_dist + BEAM_HEIGHT, xlogo_pos[2] * xlogo_dist);
		xlogo(XLOGO_RAD, color, paper_color, xlogo_alpha, xcircle);
		glPopMatrix();
	}

	glPushMatrix();
	glRotatef(beam_angle, 0, 1, 0);
	light();
	glPopMatrix();

	paper_light();

	if(show_ui) {
		ui();
	}

	if(show_help) {
		help();
	}

	glutSwapBuffers();
}

static void light()
{
	float beam_alpha = 1.0 - paper;
	if(beam_alpha <= 0.0f) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	glPushMatrix();

	glTranslatef(0, BEAM_HEIGHT, 0.2);
	bind_program(sdr_beam);
	set_uniform_float(sdr_beam, "beam_len", beam_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(0);

	for(int i=0; i<BEAM_SHELLS; i++) {
		float t = (float)i / (float)(BEAM_SHELLS - 1);
		float rad = BEAM_RMIN + (BEAM_RMAX - BEAM_RMIN) * t;
		t += 0.00001;
		float alpha = BEAM_ENERGY / (t * t);

		if(i == 0) continue;
		glColor4f(beam_color[0], beam_color[1], beam_color[2], alpha * beam_alpha);

		glutSolidCylinder(rad, beam_len, 12, 1);
	}

	glDepthMask(1);

	bind_program(0);

	glPopMatrix();

	glPopAttrib();
}

static void paper_light()
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glRotatef(beam_angle, 0, 1, 0);
	glTranslatef(0, BEAM_HEIGHT, 0.3);

	float rad = BEAM_RMIN + (BEAM_RMAX - BEAM_RMIN) * 0.6f;
	float maxdist = beam_len - 0.6f;
	float end_rad = XLOGO_RAD * 0.4;

	glLineWidth(3);
	glBegin(GL_LINES);
	glColor4f(ink_color[0], ink_color[1], ink_color[2], paper);

	glVertex3f(0, rad, 0);
	glVertex3f(0, end_rad, maxdist);
	glVertex3f(0, -rad, 0);
	glVertex3f(0, -end_rad, maxdist);
	glEnd();

	glPopMatrix();
	glPopAttrib();
}

static void backdrop(float alpha)
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CW);
	bind_program(sdr_sky);
	set_uniform_float(sdr_sky, "alpha", alpha);
	glutSolidSphere(200, 16, 32);
	bind_program(0);
	glFrontFace(GL_CCW);

	glPopAttrib();
}

static void help()
{
	static const char *help_lines[] = {
		"Camera control",
		"   LMB drag ....... rotate",
		"   MMB drag ....... pan",
		"   RMB drag/wheel . zoom",
		"   c .............. toggle free/animated camera",
		"   v .............. print current view parameters",
		"",
		"Animation control",
		"   <space> ........ pause time",
		"   <backspace> .... restart time",
		"   +/- ............ change beam rotation speed and set keyframe",
		"   0 .............. clear beam rotation keyframes",
		"   [/] ............ change beam length and set keyframe",
		"   \\ .............. clear beam length keyframes",
		"   <enter> ........ record automatic beam stop transition",
		"   K .............. set camera keyframe",
		"   <shift>-L ...... clear all camera keyframes",
		"   X/Z ............ toggle X logo/circle and set keyframe",
		"   <shift>-X/Z .... clear logo/circle keyframes",
		"   P .............. toggle paper",
		"   <shift>-P ...... clear paper keyframes",
		"   ~ .............. dump all animation keyframes to seq_dump",
		0
	};

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(0, 0);
	glVertex2f(0, win_height);
	glVertex2f(win_width, win_height);
	glVertex2f(win_width, 0);
	glEnd();
	glDisable(GL_BLEND);

	int xpos = 20;
	int ypos = 30;
	for(int i=0; help_lines[i]; i++) {
		glColor3f(0.05, 0.05, 0.05);
		glRasterPos2i(xpos + 1, ypos + 2);
		const char *s = help_lines[i];
		while(*s) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s++);
		}
		glColor3f(0.7, 1, 0.6);
		glRasterPos2i(xpos, ypos);
		s = help_lines[i];
		while(*s) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s++);
		}

		ypos += 25;
	}

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

static void idle()
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(50, (float)x / (float)y, 0.5, 500);
}

static void keyboard(unsigned char c, int x, int y)
{
	int idx;
	static Camera orig_cam;
	long anim_time = anim_stopped ? anim_stop_time - start_time : tmsec;

	switch(c) {
	case 27:
		exit(0);

	case '\b':
		start_time = glutGet(GLUT_ELAPSED_TIME);
		prev_tmsec = 0;
		anim_stop_time = anim_stopped ? start_time : 0;
		beam_angle = 0;
		break;

	case ' ':
		if(anim_stopped) {
			long msec = glutGet(GLUT_ELAPSED_TIME);
			start_time += msec - anim_stop_time;
			prev_tmsec = msec - start_time;
			anim_stop_time = 0;
			anim_stopped = false;
		} else {
			anim_stop_time = glutGet(GLUT_ELAPSED_TIME);
			anim_stopped = true;
		}
		break;

	case '=':
		beam_speed = get_seq_value("beam-speed", anim_time);
		set_seq_value("beam-speed", anim_time, beam_speed + 0.1);
		break;

	case '-':
		beam_speed = get_seq_value("beam-speed", anim_time) - 0.1;
		if(beam_speed < 0) beam_speed = 0;
		set_seq_value("beam-speed", anim_time, beam_speed);
		break;

	case '0':
		clear_seq_track("beam-speed");
		break;

	case '[':
		beam_len = get_seq_value("beam-len", anim_time) - 0.5;
		if(beam_len < 0) beam_len = 0;
		set_seq_value("beam-len", anim_time, beam_len);
		break;

	case ']':
		beam_len = get_seq_value("beam-len", anim_time);
		set_seq_value("beam-len", anim_time, beam_len + 0.5);
		break;

	case '\\':
		clear_seq_track("beam-len");
		break;

	case '\r':
	case '\n':
		idx = find_seq_track("beam-speed");
		assert(idx >= 0);
		set_seq_value(idx, anim_time, beam_speed);
		set_seq_value(idx, anim_time + 2000, 0);
		break;

	case 'c':
		freecam = !freecam;
		printf("camera mode: %s\n", freecam ? "free" : "animated");
		if(!freecam) {
			orig_cam = cam;
		} else {
			cam = orig_cam;
		}
		break;

	case 'v':
		printf("current view\n");
		printf(" pos: %f %f %f\n", cam.x, cam.y, cam.z);
		printf(" theta: %f, phi: %f\n", cam.theta, cam.phi);
		printf(" dist: %f\n", cam.dist);
		break;

	case 'L':
		printf("clearing camera keyframes\n");
		clear_seq_track("cam-x");
		clear_seq_track("cam-y");
		clear_seq_track("cam-z");
		clear_seq_track("cam-theta");
		clear_seq_track("cam-phi");
		clear_seq_track("cam-dist");
		break;

	case 'k':
		printf("setting camera keyframe for time: %ld\n", anim_time);
		set_seq_value("cam-x", anim_time, cam.x);
		set_seq_value("cam-y", anim_time, cam.y);
		set_seq_value("cam-z", anim_time, cam.z);
		set_seq_value("cam-theta", anim_time, cam.theta);
		set_seq_value("cam-phi", anim_time, cam.phi);
		set_seq_value("cam-dist", anim_time, cam.dist);
		break;

	case 'x':
		set_seq_value("xlogo", anim_time, xlogo_alpha < 0.5 ? 1.0 : 0.0);
		break;

	case 'X':
		printf("clearing logo keyframes\n");
		clear_seq_track("xlogo");
		break;

	case 'z':
		set_seq_value("xcircle", anim_time, xcircle < 0.5 ? 1.0 : 0.0);
		break;

	case 'Z':
		printf("clearing circle keyframes\n");
		clear_seq_track("xcircle");
		break;


	case 'p':
		set_seq_value("paper", anim_time, paper < 0.5 ? 1.0 : 0.0);
		break;

	case 'P':
		printf("clearing paper keyframes\n");
		clear_seq_track("paper");
		break;

	case '`':
		printf("dumping animation data to: seq_dump\n");
		if(!dump_seq("seq_dump")) {
			fprintf(stderr, "dump failed\n");
		}
		break;

	default:
		break;
	}
}

static void keyb_special(int key, int x, int y)
{
	switch(key) {
	case GLUT_KEY_F1:
		show_help = !show_help;
		break;

	case GLUT_KEY_F5:
		show_ui = !show_ui;
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
	bool pressed = state == GLUT_DOWN;
	bst[button] = pressed;

	prev_x = x;
	prev_y = y;

	if(pressed) {
		switch(bn) {
		case 3:
			cam.dist -= 0.5;
			if(cam.dist < 0) cam.dist = 0;
			break;

		case 4:
			cam.dist += 0.5;
			break;

		default:
			break;
		}
	}
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
		cam.theta += dx * 0.5;
		cam.phi += dy * 0.5;

		if (cam.phi < -90)
			cam.phi = -90;

		if (cam.phi > 90)
			cam.phi = 90;
	}

	if (bst[2]) {
		cam.dist += dy * 0.1;

		if (cam.dist < 0)
			cam.dist = 0;
	}

	if(bst[1]) {
		float theta = cam.theta / 180.0f * M_PI;
		float phi = cam.phi / 180.0f * M_PI;

		float pan_u[3], pan_v[3];

		pan_u[0] = cos(theta);
		pan_u[1] = 0;
		pan_u[2] = sin(theta);

		pan_v[0] = sin(phi) * sin(theta);
		pan_v[1] = cos(phi);
		pan_v[2] = -sin(phi) * cos(theta);

		float pan_x = -dx * 0.002 * cam.dist;
		float pan_y = dy * 0.002 * cam.dist;

		cam.x += pan_u[0] * pan_x + pan_v[0] * pan_y;
		cam.y += pan_u[1] * pan_x + pan_v[1] * pan_y;
		cam.z += pan_u[2] * pan_x + pan_v[2] * pan_y;
	}
}
