#pragma once
#include <string>

struct ScriptComponent {
	std::string scriptPath = "";
};

struct Script {
	ScriptComponent component;

	// Script state
	bool initialized = false;

	// Script
	std::string script;
};