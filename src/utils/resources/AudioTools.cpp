#include "AudioTools.h"
#include "../../external/stb/stb_vorbis.c"
#include "../AudioChecks.h"

void AudioTools::load(const std::string& filePath, ALuint* buffer, ALuint* source) {
	std::string extension = FileTools::extension(filePath);
	if (extension == "wav") {
		uint8_t channels;
		int32_t sampleRate;
		uint8_t bitsPerSample;
		ALsizei size;
		std::vector<char> data;
		loadWav(filePath, &channels, &sampleRate, &bitsPerSample, &size, data);
		alCall(alGenBuffers, 1, buffer);

		ALenum format;
		if (channels == 1 && bitsPerSample == 8) {
			format = AL_FORMAT_MONO8;
		}
		else if (channels == 1 && bitsPerSample == 16) {
			format = AL_FORMAT_MONO16;
		}
		else if (channels == 2 && bitsPerSample == 8) {
			format = AL_FORMAT_STEREO8;
		}
		else if (channels == 2 && bitsPerSample == 16) {
			format = AL_FORMAT_STEREO16;
		}
		else {
			NEIGE_ERROR("Error loading audio file \"" + filePath + "\" with " + std::to_string(channels) + " channels and " + std::to_string(bitsPerSample) + " bits per sample.");
		}

		alCall(alBufferData, *buffer, format, data.data(), size, sampleRate);
	}
	else if (extension == "ogg") {
		int channels;
		int sampleRate;
		int size;
		short* data;
		loadOggVorbis(filePath, &channels, &sampleRate, &size, &data);
		alCall(alGenBuffers, 1, buffer);

		ALenum format;
		if (channels == 1) {
			format = AL_FORMAT_MONO16;
		}
		else if (channels == 2) {
			format = AL_FORMAT_STEREO16;
		}
		else {
			NEIGE_ERROR("Error loading audio file \"" + filePath + "\" with " + std::to_string(channels) + " channels.");
		}

		alCall(alBufferData, *buffer, format, data, size, sampleRate);
	}
	else {
		NEIGE_ERROR("\"." + extension + "\" audio extension not supported.");
	}

	alCall(alGenSources, 1, source);
	alCall(alSourcef, *source, AL_PITCH, 1.0f);
	alCall(alSourcef, *source, AL_GAIN, 1.0f);
	alCall(alSource3f, *source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alCall(alSource3f, *source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alCall(alSourcei, *source, AL_LOOPING, AL_FALSE);
	alCall(alSourcei, *source, AL_BUFFER, *buffer);
}

void AudioTools::loadWav(const std::string& filePath, uint8_t* channels, int32_t* sampleRate, uint8_t* bitsPerSample, ALsizei* size, std::vector<char>& data) {
	char buffer[4];

	int32_t tmp = 0;

	std::ifstream file(filePath, std::ios::binary);

	if (!file.is_open()) {
		NEIGE_ERROR("Cannot open audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read RIFF for audio file \"" + filePath + "\".");
	}
	if (strncmp(buffer, "RIFF", 4) != 0) {
		NEIGE_ERROR("File  \"" + filePath + "\" is not a valid WAVE audio file (header missing RIFF).");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read the size of audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read WAVE for audio file \"" + filePath + "\".");
	}
	if (strncmp(buffer, "WAVE", 4) != 0) {
		NEIGE_ERROR("File  \"" + filePath + "\" is not a valid WAVE audio file (header missing WAVE).");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read fmt/0 for audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read 16 for audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 2)) {
		NEIGE_ERROR("Cannot read PCM for audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 2)) {
		NEIGE_ERROR("Cannot read the number of channels for audio file \"" + filePath + "\".");
	}
	memcpy(&tmp, buffer, 2);
	*channels = static_cast<uint8_t>(tmp);

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read the sample rate for audio file \"" + filePath + "\".");
	}
	memcpy(&tmp, buffer, 4);
	*sampleRate = static_cast<int32_t>(tmp);

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read (sampleRate * bitsPerSample * channels) / 8 for audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 2)) {
		NEIGE_ERROR("Cannot read unknown data for audio file \"" + filePath + "\".");
	}

	if (!file.read(buffer, 2)) {
		NEIGE_ERROR("Cannot read bits per sample for audio file \"" + filePath + "\".");
	}
	memcpy(&tmp, buffer, 2);
	*bitsPerSample = static_cast<uint8_t>(tmp);

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read data header for audio file \"" + filePath + "\".");
	}
	if (strncmp(buffer, "data", 4) != 0) {
		NEIGE_ERROR("File  \"" + filePath + "\" is not a valid WAVE audio file (header missing data).");
	}

	if (!file.read(buffer, 4)) {
		NEIGE_ERROR("Cannot read data size for audio file \"" + filePath + "\".");
	}
	memcpy(&tmp, buffer, 4);
	*size = static_cast<ALsizei>(tmp);

	if (file.eof()) {
		NEIGE_ERROR("File  \"" + filePath + "\" is not a valid WAVE audio file (missing data).");
	}
	if (file.fail()) {
		NEIGE_ERROR("Encountered an unknown error when loading WAVE file  \"" + filePath + "\".");
	}

	data.resize(*size);
	file.read(&data[0], *size);
}

void AudioTools::loadOggVorbis(const std::string& filePath, int* channels, int* sampleRate, int* size, short** data) {
	*size = stb_vorbis_decode_filename(filePath.c_str(), channels, sampleRate, data);
	if (*size == -1) {
		NEIGE_ERROR("Encountered an unknown error when loading Ogg Vorbis file  \"" + filePath + "\".");
	}
	*size = *size * *channels * (sizeof(int16_t) / sizeof(uint8_t));
}
