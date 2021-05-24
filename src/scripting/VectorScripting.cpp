#include "VectorScripting.h"

void VectorScripting::init() {
    lua_register(L, "normalize", normalize);
    lua_register(L, "dot", dot);
    lua_register(L, "cross", cross);
}

int VectorScripting::normalize(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 2) {
        if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            float x = static_cast<float>(lua_tonumber(L, 1));
            float y = static_cast<float>(lua_tonumber(L, 2));
            glm::vec2 res = glm::normalize(glm::vec2(x, y));

            lua_pushnumber(L, res.x);
            lua_pushnumber(L, res.y);

            return 2;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"normalize(float x, float y, [float z], [float w])\" takes from 2 to 4 float parameters.");
            return 0;
        }
    }
    else if (n == 3) {
        if (lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            float x = static_cast<float>(lua_tonumber(L, 1));
            float y = static_cast<float>(lua_tonumber(L, 2));
            float z = static_cast<float>(lua_tonumber(L, 3));
            glm::vec3 res = glm::normalize(glm::vec3(x, y, z));

            lua_pushnumber(L, res.x);
            lua_pushnumber(L, res.y);
            lua_pushnumber(L, res.z);

            return 3;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"normalize(float x, float y, [float z], [float w])\" takes from 2 to 4 float parameters.");
            return 0;
        }
    }
    else if (n == 4) {
        if (lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            float x = static_cast<float>(lua_tonumber(L, 1));
            float y = static_cast<float>(lua_tonumber(L, 2));
            float z = static_cast<float>(lua_tonumber(L, 3));
            float w = static_cast<float>(lua_tonumber(L, 4));
            glm::vec4 res = glm::normalize(glm::vec4(x, y, z, w));

            lua_pushnumber(L, res.x);
            lua_pushnumber(L, res.y);
            lua_pushnumber(L, res.z);
            lua_pushnumber(L, res.w);

            return 4;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"normalize(float x, float y, [float z], [float w])\" takes from 2 to 4 float parameters.");
            return 0;
        }
    }
    else {
        NEIGE_SCRIPT_ERROR("Function \"normalize(float x, float y, [float z], [float w])\" takes from 2 to 4 float parameters.");
        return 0;
    }
}

int VectorScripting::dot(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 4) {
        if (lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            // 2 vec2
            float x1 = static_cast<float>(lua_tonumber(L, 1));
            float y1 = static_cast<float>(lua_tonumber(L, 2));
            float x2 = static_cast<float>(lua_tonumber(L, 3));
            float y2 = static_cast<float>(lua_tonumber(L, 4));
            glm::vec2 tmp1 = glm::vec2(x1, y1);
            glm::vec2 tmp2 = glm::vec2(x2, y2);

            float res = glm::dot(tmp1, tmp2);

            lua_pushnumber(L, res);

            return 1;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"dot(float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2])\" takes 4, 6 or 8 float parameters.");
            return 0;
        }
    }
    else if (n == 6) {
        if (lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            // 2 vec3
            float x1 = static_cast<float>(lua_tonumber(L, 1));
            float y1 = static_cast<float>(lua_tonumber(L, 2));
            float z1 = static_cast<float>(lua_tonumber(L, 3));
            float x2 = static_cast<float>(lua_tonumber(L, 4));
            float y2 = static_cast<float>(lua_tonumber(L, 5));
            float z2 = static_cast<float>(lua_tonumber(L, 6));
            glm::vec3 tmp1 = glm::vec3(x1, y1, z1);
            glm::vec3 tmp2 = glm::vec3(x2, y2, z2);

            float res = glm::dot(tmp1, tmp2);

            lua_pushnumber(L, res);

            return 1;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"dot(float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2])\" takes 4, 6 or 8 float parameters.");
            return 0;
        }
    }
    else if (n == 8) {
        if (lua_isnumber(L, -8) && lua_isnumber(L, -7) && lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            // 2 vec4
            float x1 = static_cast<float>(lua_tonumber(L, 1));
            float y1 = static_cast<float>(lua_tonumber(L, 2));
            float z1 = static_cast<float>(lua_tonumber(L, 3));
            float w1 = static_cast<float>(lua_tonumber(L, 4));
            float x2 = static_cast<float>(lua_tonumber(L, 5));
            float y2 = static_cast<float>(lua_tonumber(L, 6));
            float z2 = static_cast<float>(lua_tonumber(L, 7));
            float w2 = static_cast<float>(lua_tonumber(L, 8));
            glm::vec4 tmp1 = glm::vec4(x1, y1, z1, w1);
            glm::vec4 tmp2 = glm::vec4(x2, y2, z2, w2);

            float res = glm::dot(tmp1, tmp2);

            lua_pushnumber(L, res);

            return 1;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"dot(float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2])\" takes 4, 6 or 8 float parameters.");
            return 0;
        }
    }
    else {
        NEIGE_SCRIPT_ERROR("Function \"dot(float x1, float y1, [float z1], [float w1], float x2, float y2, [float z2], [float w2])\" takes 4, 6 or 8 float parameters.");
        return 0;
    }
}

int VectorScripting::cross(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 6) {
        if (lua_isnumber(L, -6) && lua_isnumber(L, -5) && lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            float x1 = static_cast<float>(lua_tonumber(L, 1));
            float y1 = static_cast<float>(lua_tonumber(L, 2));
            float z1 = static_cast<float>(lua_tonumber(L, 3));
            float x2 = static_cast<float>(lua_tonumber(L, 4));
            float y2 = static_cast<float>(lua_tonumber(L, 5));
            float z2 = static_cast<float>(lua_tonumber(L, 6));
            glm::vec3 tmp1 = glm::vec3(x1, y1, z1);
            glm::vec3 tmp2 = glm::vec3(x2, y2, z2);

            glm::vec3 res = glm::cross(tmp1, tmp2);

            lua_pushnumber(L, res.x);
            lua_pushnumber(L, res.y);
            lua_pushnumber(L, res.z);

            return 3;
        }
        else {
            NEIGE_SCRIPT_ERROR("Function \"cross(float x1, float y1, float z1, float x2, float y2, float z2)\" takes 6 float parameters.");
            return 0;
        }
    }
    else {
        NEIGE_SCRIPT_ERROR("Function \"cross(float x1, float y1, float z1, float x2, float y2, float z2)\" takes 6 float parameters.");
        return 0;
    }
}