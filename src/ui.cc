#include <stdio.h>
#include <stdarg.h>
#include <alloca.h>
#include <GL/glut.h>
#include "ui.h"

extern int win_width, win_height;

extern bool anim_stopped;
extern long anim_time;


bool init_ui()
{
	return true;
}

void destroy_ui()
{
}

void ui()
{
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

	if(anim_stopped) {
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex2f(10, 10);
		glVertex2f(10, 50);
		glVertex2f(22, 50);
		glVertex2f(22, 10);
		glVertex2f(30, 10);
		glVertex2f(30, 50);
		glVertex2f(42, 50);
		glVertex2f(42, 10);
		glEnd();
	}

	glColor3f(1, 0.9, 0.5);
	gl_printf(win_width - 100, 20, "%4ld.%03ld", anim_time / 1000, anim_time % 1000);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void gl_printf(int x, int y, const char *fmt, ...)
{
	va_list ap;
	int buf_size, curx, cury;
	char *buf, tmp;

	va_start(ap, fmt);
	buf_size = vsnprintf(&tmp, 0, fmt, ap);
	va_end(ap);

	if(buf_size == -1) {
		buf_size = 512;
	}

	buf = (char*)alloca(buf_size + 1);
	va_start(ap, fmt);
	vsnprintf(buf, buf_size + 1, fmt, ap);
	va_end(ap);

	static const float tabstop = 4;
	static const float line_spacing = 18;

	curx = x;
	cury = y;
	glRasterPos2i(x, y);

	while(*buf) {
		char c = *buf++;

		switch(c) {
		case '\r':
			if(*buf == '\n') ++buf;
		case '\n':
			cury += line_spacing;
			curx = x;
			glRasterPos2i(curx, cury);
			break;

		case '\t':
			curx = (curx / tabstop) * tabstop + tabstop;
			glRasterPos2i(curx, cury);
			break;

		default:
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
		}
	}
}
