#include <string.h>
#include <vector>
#include "seq.h"
#include "track.h"

struct SeqTrack {
	char *name;
	Track *track;
};

static std::vector<SeqTrack> tracks;

bool init_seq()
{
	return true;
}

void destroy_seq()
{
	int ntrk = tracks.size();
	for(int i=0; i<ntrk; i++) {
		delete tracks[i].track;
		delete [] tracks[i].name;
	}
	tracks.clear();
}

int add_seq_track(const char *name, InterpMode inmode, ExtrapMode exmode, float defval)
{
	int idx = find_seq_track(name);
	if(idx >= 0) return idx;

	SeqTrack st;
	st.name = new char[strlen(name) + 1];
	strcpy(st.name, name);
	st.track = new Track;
	st.track->defval = defval;
	st.track->interp = inmode;
	st.track->extrap = exmode;
	tracks.push_back(st);
	return tracks.size() - 1;
}

int find_seq_track(const char *name)
{
	int ntrk = tracks.size();
	for(int i=0; i<ntrk; i++) {
		if(strcmp(tracks[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

Track *get_seq_track(int idx)
{
	return tracks[idx].track;
}

void clear_seq_track(int idx)
{
	tracks[idx].track->clear();
}

void clear_seq_track(const char *name)
{
	int idx = find_seq_track(name);
	if(idx >= 0) {
		tracks[idx].track->clear();
	}
}

void set_seq_value(int idx, long tm, float val)
{
	tracks[idx].track->set_key(tm, val);
}

void set_seq_value(const char *name, long tm, float val)
{
	int idx = find_seq_track(name);
	if(idx >= 0) {
		tracks[idx].track->set_key(tm, val);
	}
}

float get_seq_value(int idx, long tm)
{
	return (*tracks[idx].track)(tm);
}

float get_seq_value(const char *name, long tm)
{
	int idx = find_seq_track(name);
	if(idx < 0) {
		return 0.0f;
	}
	return (*tracks[idx].track)(tm);
}

bool load_seq(const char *fname)
{
	return false;	/* TODO */
}
