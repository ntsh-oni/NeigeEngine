#include "CameraScripting.h"

void CameraScripting::init() {
	lua_register(L, "getMainCameraIndex", getMainCameraIndex);
	lua_register(L, "setMainCameraIndex", setMainCameraIndex);
	lua_register(L, "getMainCameraEntity", getMainCameraEntity);
	lua_register(L, "getCameraCount", getCameraCount);

	std::string cameraScript = FileTools::readAscii("../src/scripting/scripts/camera.lua");
	luaL_dostring(L, cameraScript.c_str());
}

int CameraScripting::getMainCameraIndex(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		bool foundCamera = false;
		int cameraId = 0;
		int i = 0;
		for (Entity camera : *cameras) {
			if (mainCamera == camera) {
				cameraId = static_cast<int>(i);
				foundCamera = true;
				break;
			}
			i++;
		}

		if (foundCamera) {
			lua_pushnumber(L, cameraId);

			return 1;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getMainCameraIndex()\": unable to find main camera index.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getMainCameraIndex()\" takes no parameter.");
		return 0;
	}
}

int CameraScripting::setMainCameraIndex(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			size_t cameraId = static_cast<size_t>(lua_tonumber(L, 1));

			if (cameraId >= 0 && cameraId < cameras->size()) {
				mainCamera = *std::next(cameras->begin(), cameraId);

				return 0;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"setMainCameraIndex(int cameraId)\": cameraId must be between 0 and getCameraCount() - 1.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"setMainCameraIndex(int cameraId)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"setMainCameraIndex(int cameraId)\" takes 1 integer parameter.");
		return 0;
	}
}

int CameraScripting::getMainCameraEntity(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, mainCamera);

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getMainCameraEntity()\" takes no parameter.");
		return 0;
	}
}

int CameraScripting::getCameraCount(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 0) {
		lua_pushnumber(L, static_cast<int>(cameras->size()));

		return 1;
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getCameraCount()\" takes no parameter.");
		return 0;
	}
}