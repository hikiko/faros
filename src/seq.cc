#include <stdio.h>
#include <string.h>
#include <vector>
#include <treestore.h>
#include "seq.h"
#include "track.h"

struct SeqTrack {
	char *name;
	Track *track;
};

static std::vector<SeqTrack> tracks;


static Track *load_track(struct ts_node *tnode);
static InterpMode str2interp(const char *str);
static ExtrapMode str2extrap(const char *str);


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
	tracks[idx].track->set_value(tm, val);
}

void set_seq_value(const char *name, long tm, float val)
{
	int idx = find_seq_track(name);
	if(idx >= 0) {
		tracks[idx].track->set_value(tm, val);
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
	struct ts_node *tree;
	if(!(tree = ts_load(fname))) {
		return false;
	}
	if(strcmp(tree->name, "faros") != 0) {
		fprintf(stderr, "invalid sequence file: %s\n", fname);
		ts_free_tree(tree);
		return false;
	}

	struct ts_node *node = tree->child_list;
	while(node) {
		if(strcmp(node->name, "track") == 0) {
			Track *track = load_track(node);
			const char *name = ts_get_attr_str(node, "name", 0);
			if(!name) {
				char buf[64];
				static int foo;
				sprintf(buf, "unnamed%03d", foo++);
				name = buf;
			}

			int idx = find_seq_track(name);
			if(idx == -1) {
				SeqTrack st;
				st.track = track;
				st.name = new char[strlen(name) + 1];
				strcpy(st.name, name);
				tracks.push_back(st);
			} else {
				delete tracks[idx].track;
				tracks[idx].track = track;
			}
		}

		node = node->next;
	}

	ts_free_tree(tree);
	return true;
}

static Track *load_track(struct ts_node *tnode)
{
	Track *track = new Track;
	track->defval = ts_get_attr_num(tnode, "default", 0.0f);
	track->interp = str2interp(ts_get_attr_str(tnode, "interpolation", ""));
	track->extrap = str2extrap(ts_get_attr_str(tnode, "extrapolation", ""));

	struct ts_node *keynode = tnode->child_list;
	while(keynode) {
		long tm = ts_get_attr_int(keynode, "time", 0);
		float val = ts_get_attr_num(keynode, "value", 0.0f);
		track->set_value(tm, val);
		keynode = keynode->next;
	}

	return track;
}

static InterpMode str2interp(const char *str)
{
	if(strcmp(str, "step") == 0) {
		return INTERP_STEP;
	} else if(strcmp(str, "sigmoid") == 0) {
		return INTERP_SIGMOID;
	}
	return INTERP_LINEAR;
}

static ExtrapMode str2extrap(const char *str)
{
	if(strcmp(str, "repeat") == 0) {
		return EXTRAP_REPEAT;
	}
	return EXTRAP_CLAMP;
}

bool dump_seq(const char *fname)
{
	FILE *fp = fopen(fname, "w");
	if(!fp) {
		perror("failed to open sequence dump file");
		return false;
	}

	fprintf(fp, "faros {\n");
	int ntrk = tracks.size();
	for(int i=0; i<ntrk; i++) {
		fprintf(fp, "  track {\n");
		fprintf(fp, "    name = \"%s\"\n", tracks[i].name);

		int nkeys = tracks[i].track->get_num_keys();
		for(int j=0; j<nkeys; j++) {
			TrackKey key = (*tracks[i].track)[j];
			fprintf(fp, "    key {\n");
			fprintf(fp, "      time = %ld\n", key.time);
			fprintf(fp, "      value = %g\n", key.value);
			fprintf(fp, "    }\n");
		}
		fprintf(fp, "  }\n\n");
	}
	fprintf(fp, "}\n");
	fclose(fp);
	return true;
}
