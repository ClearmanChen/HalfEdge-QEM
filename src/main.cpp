#include "Window.h"
#include "ObjLoader.h"

#define GLFW_WINDOW_WIDTH 800
#define GLFW_WINDOW_HEIGHT 600

int main() 
{
	Window window(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT);
	window.run();
	return 0;
}