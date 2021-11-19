#include "Audio.h"
#include "../utils/AudioChecks.h"
#include "../utils/resources/AudioTools.h"

extern ECS ecs;

void Audio::init() {
	// Open audio device
	device = alcOpenDevice(nullptr);
	if (!device) {
		NEIGE_ERROR("Unable to open audio device.");
	}

	// Create context
	if (!alcCall(alcCreateContext, context, device, device, nullptr) || !context) {
		NEIGE_ERROR("Unable to create audio context.");
	}

	// Make context current
	ALCboolean makeContextCurrent = false;
	if (!alcCall(alcMakeContextCurrent, makeContextCurrent, device, context) || (makeContextCurrent != ALC_TRUE)) {
		NEIGE_ERROR("Unable to make the audio context current.");
	}
}

void Audio::update() {
	for (Sound& sound : sounds) {
		sound.update();
	}
}

void Audio::destroy() {
	for (Sound& sound : sounds) {
		if (sound.isPlaying()) {
			sound.stop();
		}
		sound.destroy();
	}
	alcMakeContextCurrent(context);
	alcDestroyContext(context);
	alcCloseDevice(device);
}
