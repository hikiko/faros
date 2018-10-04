#include <assert.h>
#include <algorithm>
#include "track.h"

static long remap_time(ExtrapMode mode, long t, long t0, long t1);
static float smoothstep(float a, float b, float x);

Track::Track()
{
	interp = INTERP_SIGMOID;
	extrap = EXTRAP_CLAMP;
	keys_sorted = true;
	defval = 0.0f;
}

void Track::clear()
{
	keys.clear();
}

bool Track::empty() const
{
	return keys.empty();
}

int Track::get_num_keys() const
{
	return (int)keys.size();
}

const TrackKey &Track::operator [](int idx) const
{
	return keys[idx];
}

TrackKey &Track::operator [](int idx)
{
	return keys[idx];
}

void Track::set_value(long tm, float val)
{
	int idx = find_key(tm);
	if(idx >= 0) {
		keys[idx].value = val;
		return;
	}

	TrackKey key = {tm, val};
	keys.push_back(key);
	keys_sorted = false;
}

float Track::get_value(long tm) const
{
	int idx = find_key(tm);
	if(idx == -1) return 0.0f;
	return keys[idx].value;
}

int Track::find_key(long tm) const
{
	int sz = (int)keys.size();
	for(int i=0; i<sz; i++) {
		if(keys[i].time == tm) return i;
	}
	return -1;
}

static bool keycmp(const TrackKey &a, const TrackKey &b)
{
	return a.time < b.time;
}

float Track::operator ()(long tm) const
{
	if(keys.empty()) {
		return defval;
	}

	int nkeys = keys.size();
	if(nkeys == 1) {
		return keys[0].value;
	}
	if(!keys_sorted) {
		Track *track = (Track*)this;
		std::sort(track->keys.begin(), track->keys.end(), keycmp);
		keys_sorted = true;
	}

	long tstart = keys[0].time;
	long tend = keys[nkeys - 1].time;

	tm = remap_time(extrap, tm, tstart, tend);

	int idx0 = get_interval(tm);
	assert(idx0 >= 0 && idx0 < nkeys);
	int idx1 = idx0 + 1;

	if(idx0 == nkeys - 1) {
		return keys[idx0].value;
	}

	float dt = (float)(keys[idx1].time - keys[idx0].time);
	float t = (float)(tm - keys[idx0].time) / dt;

	switch(interp) {
	case INTERP_STEP:
		return keys[idx0].value;

	case INTERP_SIGMOID:
		t = smoothstep(0, 1, t);
	case INTERP_LINEAR:
		return keys[idx0].value + (keys[idx1].value - keys[idx0].value) * t;

	default:
		break;
	}
	return 0.0f;
}

int Track::get_interval(long tm) const
{
	int nkeys = (int)keys.size();

	for(int i=0; i<nkeys-1; i++) {
		if(tm < keys[i + 1].time) {
			return i;
		}
	}
	return nkeys - 1;
}

static long remap_time(ExtrapMode mode, long t, long t0, long t1)
{
	long interval = t1 - t0;

	switch(mode) {
	case EXTRAP_CLAMP:
		if(interval <= 0) return t0;
		return t < t0 ? t0 : (t >= t1 ? t1 : t);

	case EXTRAP_REPEAT:
		if(interval > 0) {
			long x = (t - t0) % interval;
			if(x < 0) x += interval;
			return x + t0;
		}
		return t0;

	default:
		break;
	}

	assert(!"unreachable");
	return t;
}

static float smoothstep(float a, float b, float x)
{
	if(x < a) return 0.0f;
	if(x >= b) return 1.0f;

	x = (x - a) / (b - a);
	return x * x * (3.0f - 2.0f * x);
}
