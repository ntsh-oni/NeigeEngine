#include "UIScripting.h"
#include "../graphics/resources/UIResources.h"
#include "../utils/resources/FileTools.h"
#include "../utils/resources/ImageTools.h"

void UIScripting::init() {
    lua_register(L, "createFont", createFont);
    lua_register(L, "drawText", drawText);

	std::string uiScript = FileTools::readAscii("../src/scripting/scripts/ui.lua");
	luaL_dostring(L, uiScript.c_str());
}

int UIScripting::createFont(lua_State* L) {
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
				NEIGE_SCRIPT_ERROR("Function \"createFont(string fontPath, float height)\": font \"" + fontPath + "\" does not exist.");
				return 0;
			}
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"createFont(string fontPath, float height)\" takes 1 string and 1 float parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"createFont(string fontPath, float height)\" takes 1 string and 1 float parameters.");
		return 0;
	}
}

int UIScripting::drawText(lua_State* L) {
	int n = lua_gettop(L);
	if (n == 7) {
		if (lua_isstring(L, -7) && lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			std::string t = lua_tostring(L, 1);
			int fontIndex = static_cast<int>(lua_tonumber(L, 2));
			float red = static_cast<float>(lua_tonumber(L, 3));
			float green = static_cast<float>(lua_tonumber(L, 4));
			float blue = static_cast<float>(lua_tonumber(L, 5));
			float x = static_cast<float>(lua_tonumber(L, 6));
			float y = static_cast<float>(lua_tonumber(L, 7));

			Text text;
			text.text = t;
			text.fontIndex = fontIndex;
			text.color = glm::vec3(red, green, blue);
			text.position = glm::vec2(x, y);

			texts.push(text);

			return 0;
		}
		else {
			NEIGE_SCRIPT_ERROR("Function \"drawText(string text, int fontIndex, float textColorRed, float textColorGreen, float textColorBlue, float x, float y)\" takes 1 string, 1 integer and 5 floats parameters.");
			return 0;
		}
	}
	else {
		NEIGE_SCRIPT_ERROR("Function \"drawText(string text, int fontIndex, float textColorRed, float textColorGreen, float textColorBlue, float x, float y)\" takes 1 string, 1 integer and 5 floats parameters.");
		return 0;
	}
}