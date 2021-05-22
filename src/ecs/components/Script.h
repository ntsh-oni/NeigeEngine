#pragma once
#include <string>

struct Script {
	// Component parameters
	std::string scriptPath = "";

	// Script state
	bool initialized = false;

	// Script
	std::string script;
};