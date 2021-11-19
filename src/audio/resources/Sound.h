#pragma once
#include "../../external/openal-soft/include/AL/al.h"
#include <string>

struct Sound {
	ALuint buffer;
	ALuint source;
	ALint state;

	void update();
	void destroy();
	void play();
	void pause();
	void stop();
	void setGain(float newGain);
	void setPitch(float newPitch);
	bool isPlaying();
};