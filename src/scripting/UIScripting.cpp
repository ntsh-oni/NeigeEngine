#include "UIScripting.h"
#include "../graphics/resources/UIResources.h"
#include "../utils/resources/FileTools.h"
#include "../utils/resources/ImageTools.h"

void UIScripting::init() {
	lua_register(L, "loadSprite", loadSprite);
    lua_register(L, "loadFont", loadFont);
    lua_register(L, "drawSprite", drawSprite);
    lua_register(L, "drawText", drawText);
    lua_register(L, "drawRectangle", drawRectangle);
    lua_register(L, "getSpriteSize", getSpriteSize);

	std::string uiScript = FileTools::readAscii("../src/scripting/scripts/ui.lua");
	luaL_dostring(L, uiScript.c_str());
}

int UIScripting::loadSprite(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isstring(L, -1)) {
			std::string spritePath = lua_tostring(L, 1);

			if (FileTools::exists(spritePath)) {
				SpriteImage spriteImage;
				ImageTools::loadSprite(spritePath, &spriteImage);

				spritesImages.push_back(spriteImage);

				for (uint32_t i = 0; i < framesInFlight; i++) {
					spriteDescriptorSetUpToDate[i] = false;
				}

				lua_pushnumber(L, static_cast<int>(spritesImages.size() - 1));

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"loadSprite(string spritePath)\": sprite \"" + spritePath + "\" does not exist.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"loadSprite(string spritePath)\" takes 1 string parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"loadSprite(string spritePath)\" takes 1 string parameter.");
		return 0;
	}
}

int UIScripting::loadFont(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 2) {
		if (lua_isstring(L, -2) && lua_isnumber(L, -1)) {
			std::string fontPath = lua_tostring(L, 1);
			float height = static_cast<float>(lua_tonumber(L, 2));

			if (FileTools::exists(fontPath)) {
				Font font;
				ImageTools::loadFont(fontPath, height, &font);

				fonts.push_back(font);

				for (uint32_t i = 0; i < framesInFlight; i++) {
					fontDescriptorSetUpToDate[i] = false;
				}

				lua_pushnumber(L, static_cast<int>(fonts.size() - 1));

				return 1;
			}
			else {
				NEIGE_SCRIPT_ERROR("Function \"loadFont(string fontPath, float height)\": font \"" + fontPath + "\" does not exist.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"loadFont(string fontPath, float height)\" takes 1 string and 1 float parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"loadFont(string fontPath, float height)\" takes 1 string and 1 float parameters.");
		return 0;
	}
}

int UIScripting::drawSprite(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 4) {
		if (lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			int spriteId = static_cast<int>(lua_tonumber(L, 1));

			if (spriteId > spritesImages.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"drawSprite(int spriteIndex, float positionX, float positionY, float opacity)\": spriteIndex should be inferior to the number of loaded sprites (" + std::to_string(spritesImages.size()) + ").");
				return 0;
			}

			float x = static_cast<float>(lua_tonumber(L, 2));
			float y = static_cast<float>(lua_tonumber(L, 3));
			float opacity = static_cast<float>(lua_tonumber(L, 4));

			UISprite sprite;
			sprite.spriteIndex = spriteId;
			sprite.position = glm::vec2(x, y);
			sprite.opacity = opacity;

			sprites.push(sprite);

			elementsToDraw.push(UIElement::SPRITE);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"drawSprite(int spriteIndex, float positionX, float positionY, float opacity)\" takes 1 integer, 3 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"drawSprite(int spriteIndex, float positionX, float positionY, float opacity)\" takes 1 integer, 3 floats parameters.");
		return 0;
	}
}

int UIScripting::drawText(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 8) {
		if (lua_isstring(L, -8) && lua_isnumber(L, -7) && lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			std::string t = lua_tostring(L, 1);
			int fontId = static_cast<int>(lua_tonumber(L, 2));

			if (fontId > fonts.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"drawText(string text, int fontIndex, float positionX, float positionY, float textColorRed, float textColorGreen, float textColorBlue, float textColorAlpha)\": fontIndex should be inferior to the number of loaded fonts (" + std::to_string(fonts.size()) + ").");
				return 0;
			}

			float x = static_cast<float>(lua_tonumber(L, 3));
			float y = static_cast<float>(lua_tonumber(L, 4));
			float red = static_cast<float>(lua_tonumber(L, 5));
			float green = static_cast<float>(lua_tonumber(L, 6));
			float blue = static_cast<float>(lua_tonumber(L, 7));
			float alpha = static_cast<float>(lua_tonumber(L, 8));

			UIText text;
			text.text = t;
			text.fontIndex = fontId;
			text.position = glm::vec2(x, y);
			text.color = glm::vec4(red, green, blue, alpha);

			texts.push(text);

			elementsToDraw.push(UIElement::TEXT);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"drawText(string text, int fontIndex, float positionX, float positionY, float textColorRed, float textColorGreen, float textColorBlue, float textColorAlpha)\" takes 1 string, 1 integer and 6 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"drawText(string text, int fontIndex, float positionX, float positionY, float textColorRed, float textColorGreen, float textColorBlue, float textColorAlpha)\" takes 1 string, 1 integer and 6 floats parameters.");
		return 0;
	}
}

int UIScripting::drawRectangle(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 8) {
		if (lua_isnumber(L, -8) && lua_isnumber(L, -7) && lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			float x = static_cast<float>(lua_tonumber(L, 1));
			float y = static_cast<float>(lua_tonumber(L, 2));
			float width = static_cast<float>(lua_tonumber(L, 3));
			float height = static_cast<float>(lua_tonumber(L, 4));
			float red = static_cast<float>(lua_tonumber(L, 5));
			float green = static_cast<float>(lua_tonumber(L, 6));
			float blue = static_cast<float>(lua_tonumber(L, 7));
			float alpha = static_cast<float>(lua_tonumber(L, 8));

			UIRectangle rectangle;
			rectangle.position = glm::vec2(x, y);
			rectangle.size = glm::vec2(width, height);
			rectangle.color = glm::vec4(red, green, blue, alpha);

			rectangles.push(rectangle);

			elementsToDraw.push(UIElement::RECTANGLE);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"drawRectangle(float positionX, float positionY, float width, float height, float rectangleColorRed, float rectangleColorGreen, float rectangleColorBlue, float rectangleColorAlpha)\" takes 8 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"drawRectangle(float positionX, float positionY, float width, float height, float rectangleColorRed, float rectangleColorGreen, float rectangleColorBlue, float rectangleColorAlpha)\" takes 8 floats parameters.");
		return 0;
	}
}

int UIScripting::getSpriteSize(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 1) {
		if (lua_isnumber(L, -1)) {
			int spriteId = static_cast<int>(lua_tonumber(L, 1));

			if (spriteId > spritesImages.size() - 1) {
				NEIGE_SCRIPT_ERROR("Function \"getSpriteSize(int spriteIndex)\": spriteIndex should be inferior to the number of loaded sprites (" + std::to_string(spritesImages.size()) + ").");
				return 0;
			}

			lua_pushnumber(L, spritesImages[spriteId].width);
			lua_pushnumber(L, spritesImages[spriteId].height);

			return 2;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"getSpriteSize(int spriteIndex)\" takes 1 integer parameter.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"getSpriteSize(int spriteIndex)\" takes 1 integer parameter.");
		return 0;
	}
}
