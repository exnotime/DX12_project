#include "Window.h"
#include <GLFW/glfw3.h>
Window::Window() {

}

Window::~Window() {
	glfwDestroyWindow(m_Window);
	m_Window = nullptr;
}

Window& Window::GetInstance() {
	static Window instance;
	return instance;
}

void Window::Initialize(const WindowSettings& windowSettings) {
	m_WindowSettings = windowSettings;
	if (m_Window) {
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, m_WindowSettings.Resizeable ? true : false);
	glfwWindowHint(GLFW_DECORATED, m_WindowSettings.BorderLess ? false : true);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(m_WindowSettings.Width, m_WindowSettings.Height, m_WindowSettings.Title.c_str(), nullptr, nullptr);
	if (m_Window == nullptr) {
		printf("error creating window\n");
		return;
	}
}

GLFWwindow* Window::GetWindow() const {
	return m_Window;
}

const WindowSettings& Window::GetWindowSettings() const {
	return m_WindowSettings;
}

void Window::MakeCurrent() {
	glfwMakeContextCurrent(m_Window);
}
