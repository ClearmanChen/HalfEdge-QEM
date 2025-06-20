#pragma once
#include "gl_header.h"
#include <iostream>
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Simplification.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using std::cerr;
using std::endl;

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

class Window
{
public:
	Window(const int& glfw_width, const int& glfw_height);
	~Window();
	void run();
	void processInput();

	Camera* camera;
	Mesh* mesh;
	bool mouse_flag;
	double mouse_lastX;
	double mouse_lastY;
	double mouse_offsetX;
	double mouse_offsetY;

private:
	GLFWwindow* glfw_window = nullptr;
	
	int glfw_width, glfw_height;
	float deltaTime = 0.0f;
	//Initialize GLFW window pointer, return true if successed
	bool init_glfw();
};