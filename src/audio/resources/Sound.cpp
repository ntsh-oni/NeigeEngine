#include "Sound.h"
#include "../../utils/AudioChecks.h"

void Sound::update() {
	if (isPlaying()) {
		alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
	}
}

void Sound::destroy() {
	alCall(alDeleteSources, 1, &source);
	alCall(alDeleteBuffers, 1, &buffer);
}

void Sound::play() {
	alCall(alSourcePlay, source);
	state = AL_PLAYING;
}

void Sound::stop() {
	alCall(alSourceStop, source);
	state = AL_STOPPED;
}

void Sound::setGain(float newGain) {
	alCall(alSourcef, source, AL_GAIN, newGain);
}

void Sound::setPitch(float newPitch) {
	alCall(alSourcef, source, AL_PITCH, newPitch);
}

void Sound::pause() {
	alCall(alSourcePause, source);
	state = AL_PAUSED;
}

bool Sound::isPlaying(){
	return state == AL_PLAYING;
}