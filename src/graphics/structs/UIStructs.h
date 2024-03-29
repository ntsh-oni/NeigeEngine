#pragma once
#include "../../external/glm/glm/glm.hpp"
#include "../../external/stb/stb_truetype.h"
#include "../resources/Image.h"
#include <string>

// UI elements
enum struct UIElement {
	SPRITE,
	TEXT,
	RECTANGLE
};

// Sprite
struct SpriteImage {
	Image image;
	int width;
	int height;
};

struct UISprite {
	int spriteIndex;
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	float opacity = 1.0f;
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
struct UIText {
	std::string text = "";
	int fontIndex;
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

// Rectangle
struct UIRectangle {
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec2 size = glm::vec2(0.0f, 0.0f);
	glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
};