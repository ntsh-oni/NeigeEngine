#pragma once
#include "../../external/glm/glm/glm.hpp"
#include "../../external/stb/stb_truetype.h"
#include "../resources/Image.h"
#include <string>

// Sprite
struct SpriteImage {
	Image image;
	int width;
	int height;
};

struct Sprite {
	int spriteIndex;
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
};

// Font
struct Font {
	Image image;
	char startChar;
	char endChar;
	float height;
	stbtt_bakedchar backedChar[96];
};

// Text
struct Text {
	std::string text = "";
	int fontIndex;
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
};