#include "AudioScripting.h"
#include "../utils/resources/FileTools.h"
#include "../utils/resources/AudioTools.h"

void AudioScripting::init() {
	lua_register(L, "loadAudio", loadAudio);
	lua_register(L, "playSound", playSound);
	lua_register(L, "stopSound", stopSound);
	lua_register(L, "pauseSound", pauseSound);
	lua_register(L, "setGain", setGain);
	lua_register(L, "setPitch", setPitch);

	std::string audioScript = FileTools::readAscii("../src/scripting/scripts/audio.lua");
	luaL_dostring(L, audioScript.c_str());
}

int AudioScripting::loadAudio(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isstring(L, -1)) {
			std::string soundPath = lua_tostring(L, 1);

			if (FileTools::exists(soundPath)) {
				Sound sound;
				AudioTools::load(soundPath, &sound.buffer, &sound.source);
				sound.state = AL_INITIAL;

				sounds.push_back(sound);

				lua_pushnumber(L, static_cast<int>(sounds.size() - 1));

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"loadAudio(string soundPath)\": sound \"" + soundPath + "\" does not exist.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"loadAudio(string soundPath)\" takes 1 string parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"loadAudio(string soundPath)\" takes 1 string parameter.");
		return 0;
	}
}

int AudioScripting::playSound(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int soundId = static_cast<int>(lua_tonumber(L, 1));

			if (soundId > sounds.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"playSound(int soundIndex)\": soundIndex should be inferior to the number of loaded sounds (" + std::to_string(sounds.size()) + ").");
				return 0;
			}

			sounds[soundId].play();
			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"playSound(int soundIndex)\" takes 1 int parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"playSound(int soundIndex)\" takes 1 int parameter.");
		return 0;
	}
}

int AudioScripting::stopSound(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int soundId = static_cast<int>(lua_tonumber(L, 1));

			if (soundId > sounds.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"stopSound(int soundIndex)\": soundIndex should be inferior to the number of loaded sounds (" + std::to_string(sounds.size()) + ").");
				return 0;
			}

			if (sounds[soundId].isPlaying()) {
				sounds[soundId].stop();
			}
			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"stopSound(int soundIndex)\" takes 1 int parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"stopSound(int soundIndex)\" takes 1 int parameter.");
		return 0;
	}
}

int AudioScripting::pauseSound(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int soundId = static_cast<int>(lua_tonumber(L, 1));

			if (soundId > sounds.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"pauseSound(int soundIndex)\": soundIndex should be inferior to the number of loaded sounds (" + std::to_string(sounds.size()) + ").");
				return 0;
			}

			if (sounds[soundId].isPlaying()) {
				sounds[soundId].pause();
			}
			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"pauseSound(int soundIndex)\" takes 1 int parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"pauseSound(int soundIndex)\" takes 1 int parameter.");
		return 0;
	}
}

int AudioScripting::setGain(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			int soundId = static_cast<int>(lua_tonumber(L, 1));
			float newGain = static_cast<float>(lua_tonumber(L, 2));

			if (soundId > sounds.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"setGain(int soundIndex, float newGain)\": soundIndex should be inferior to the number of loaded sounds (" + std::to_string(sounds.size()) + ").");
				return 0;
			}
			if (newGain < 0.0) {
				NEIGE_SCRIPT_ERROR("Function \"setGain(int soundIndex, float newGain)\": gain cannot be lower than 0.");
				return 0;
			}

			sounds[soundId].setGain(newGain);
			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setGain(int soundIndex, float newGain)\" takes 1 int and 1 float parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setGain(int soundIndex, float newGain)\" takes 1 int and 1 float parameter.");
		return 0;
	}
}

int AudioScripting::setPitch(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			int soundId = static_cast<int>(lua_tonumber(L, 1));
			float newPitch = static_cast<float>(lua_tonumber(L, 2));

			if (soundId > sounds.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"setPitch(int soundIndex, float newPitch)\": soundIndex should be inferior to the number of loaded sounds (" + std::to_string(sounds.size()) + ").");
				return 0;
			}
			if (newPitch < 0.0) {
				NEIGE_SCRIPT_ERROR("Function \"setPitch(int soundIndex, float newPitch)\": pitch cannot be lower than 0.");
				return 0;
			}

			sounds[soundId].setPitch(newPitch);
			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setPitch(int soundIndex, float newPitch)\" takes 1 int and 1 float parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setPitch(int soundIndex, float newPitch)\" takes 1 int and 1 float parameter.");
		return 0;
	}
}