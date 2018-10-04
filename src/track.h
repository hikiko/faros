#ifndef TRACK_H_
#define TRACK_H_

#include <vector>

enum InterpMode {
	INTERP_STEP,
	INTERP_LINEAR,
	INTERP_SIGMOID
};

enum ExtrapMode {
	EXTRAP_CLAMP,
	EXTRAP_REPEAT
};

struct TrackKey {
	long time;
	float value;
};

class Track {
private:
	std::vector<TrackKey> keys;
	mutable bool keys_sorted;

	int get_interval(long tm) const;

public:
	InterpMode interp;
	ExtrapMode extrap;
	float defval;

	Track();

	void clear();
	bool empty() const;

	int get_num_keys() const;

	const TrackKey &operator [](int idx) const;
	TrackKey &operator [](int idx);

	void set_value(long tm, float val);
	float get_value(long tm) const;
	int find_key(long tm) const;

	float operator ()(long tm) const;
};

#endif	/* TRACK_H_ */
