#pragma once
#include "../../external/openal-soft/include/AL/al.h"
#include "../../utils/NeigeDefines.h"
#include "../../utils/resources/FileTools.h"
#include <cstdint>
#include <fstream>
#include <string>

struct AudioTools {
	static void load(const std::string& filePath, ALuint* buffer, ALuint* source);
	static void loadWav(const std::string& filePath, uint8_t* channels, int32_t* sampleRate, uint8_t* bitsPerSample, ALsizei* size, std::vector<char>& data);
	static void loadOggVorbis(const std::string& filePath, int* channels, int* sampleRate, int* size, short** data);
};