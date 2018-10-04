#ifndef ANIM_H_
#define ANIM_H_

#include "track.h"

bool init_seq();
void destroy_seq();

int add_seq_track(const char *name, InterpMode inmode, ExtrapMode exmode, float defval);
int find_seq_track(const char *name);
Track *get_seq_track(int idx);

void clear_seq_track(int idx);
void clear_seq_track(const char *name);

void set_seq_value(int idx, long tm, float val);
void set_seq_value(const char *name, long tm, float val);
float get_seq_value(int idx, long tm);
float get_seq_value(const char *name, long tm);

bool load_seq(const char *fname);

#endif	/* ANIM_H_ */
