#pragma once
#include <string>

enum struct SkyboxType {
	NONE,
	ENVMAP
};

struct Scene {
	SkyboxType skyboxType = SkyboxType::NONE;
	std::string envmapPath = "";
};