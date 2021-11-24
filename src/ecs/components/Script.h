#pragma once
#include <string>

struct ScriptComponent {
	std::string scriptPath = "";
};

struct Script {
	ScriptComponent component;

	// State
	bool initialized = false;
	bool destroyed = false;
	bool justDestroyed = false;
	bool canBeDestroyedNow = true;

	// Script
	std::string script;
};