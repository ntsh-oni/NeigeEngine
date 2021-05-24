#pragma once
#define GLFW_INCLUDE_VULKAN
#include "../../external/glfw/include/GLFW/glfw3.h"
#include "../utils/NeigeDefines.h"
#include "../graphics/structs/RendererStructs.h"
#include "../inputs/Inputs.h"
#include "Surface.h"
#include <string>
#include <vector>

struct Window {
	// Callback when window get resized
	static void framebufferResizeCallback(GLFWwindow* window, [[maybe_unused]] int width, [[maybe_unused]] int height) {
		auto pointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pointer->windowGotResized();
	}

	// Callback for keyboard keys
	static void keyCallback(GLFWwindow* window, int key, [[maybe_unused]]  int scancode, int action, [[maybe_unused]] int mods) {
		auto pointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pointer->keyAction(key, action);
	}

	// Callback for mouse buttons
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, [[maybe_unused]] int mods) {
		auto pointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));;
		pointer->mouseButtonAction(button, action);
	}

	// Callback for mouse position
	static void mousePositionCallback(GLFWwindow* window, double x, double y) {
		auto pointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pointer->mousePositionAction(x, y);
	}

	GLFWwindow* window;
	Surface surface;
	VkExtent2D extent;
	GLFWmonitor* monitor;
	bool gotResized = false;

	int oldXPos;
	int oldYPos;
	VkExtent2D oldExtent;

	void init(const std::string& applicationName);
	void destroy();
	void updateExtent();
	std::vector<const char*> instanceExtensions();
	void createSurface();
	void windowGotResized();
	void keyAction(int key, int action);
	void mouseButtonAction(int button, int action);
	void mousePositionAction(double x, double y);
	void closeWindow();
	bool isFullscreen();
	void setFullscreen(bool fullscreen);
	void showCursor(bool show);
	void setCursorPosition(double x, double y);
	void setWindowTitle(const std::string& title);
	void setWindowSize(int width, int height);
	bool windowGotClosed();
	void pollEvents();
	void waitEvents();
};