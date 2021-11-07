#pragma once
#include <string>

enum struct SkyboxType {
	ATMOSPHERE,
	ENVMAP
};

struct Scene {
	SkyboxType skyboxType = SkyboxType::ATMOSPHERE;
	std::string envmapPath = "";
};