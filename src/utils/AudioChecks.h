#pragma once
#include "../../external/openal-soft/include/AL/al.h"
#include "../../external/openal-soft/include/AL/alc.h"
#include "../utils/NeigeDefines.h"
#include <iostream>

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

inline bool alCheckErrors(const std::string& filename, const uint32_t line) {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		switch (error) {
		case AL_INVALID_NAME:
			NEIGE_AUDIO_ERROR("AL error: Name is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case AL_INVALID_ENUM:
			NEIGE_AUDIO_ERROR("AL error: Enum is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case AL_INVALID_VALUE:
			NEIGE_AUDIO_ERROR("AL error: Value is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case AL_INVALID_OPERATION:
			NEIGE_AUDIO_ERROR("AL error: Operation is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case AL_OUT_OF_MEMORY:
			NEIGE_AUDIO_ERROR("AL error: Out of memory.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		default:
			NEIGE_AUDIO_ERROR("AL error: Unknown error.\nFile: " + filename + "\nLine: " + std::to_string(line));
		}
		return false;
	}
	return true;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const uint32_t line, alFunction function, Params... params)
->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))> {
	auto r = function(std::forward<Params>(params)...);
	alCheckErrors(filename, line);
	return r;
}

template<typename alFunction, typename... Params>
auto alCallImpl(const char* filename, const uint32_t line, alFunction function, Params... params)
->typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool> {
	function(std::forward<Params>(params)...);
	return alCheckErrors(filename, line);
}

#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

inline bool alcCheckErrors(const std::string& filename, const uint32_t line, ALCdevice* device) {
	ALCenum error = alcGetError(device);
	if (error != ALC_NO_ERROR) {
		switch (error) {
		case ALC_INVALID_VALUE:
			NEIGE_AUDIO_ERROR("ALC error: Value is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case ALC_INVALID_DEVICE:
			NEIGE_AUDIO_ERROR("ALC error: Device is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case ALC_INVALID_CONTEXT:
			NEIGE_AUDIO_ERROR("ALC error: Context is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case ALC_INVALID_ENUM:
			NEIGE_AUDIO_ERROR("ALC error: Enum is invalid.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		case ALC_OUT_OF_MEMORY:
			NEIGE_AUDIO_ERROR("ALC error: Out of memory.\nFile: " + filename + "\nLine: " + std::to_string(line));
			break;
		default:
			NEIGE_AUDIO_ERROR("ALC error: Unknown error.\nFile: " + filename + "\nLine: " + std::to_string(line));
		}
		return false;
	}
	return true;
}

template<typename alcFunction, typename... Params>
auto alcCallImpl(const char* filename, const uint32_t line, alcFunction function, ALCdevice* device, Params... params)
->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool> {
	function(std::forward<Params>(params)...);
	return alcCheckErrors(filename, line, device);
}

template<typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char* filename, const uint32_t line, alcFunction function, ReturnType& returnValue, ALCdevice* device, Params... params)
->typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool> {
	returnValue = function(std::forward<Params>(params)...);
	return alcCheckErrors(filename, line, device);
}