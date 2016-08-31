#include "Input.h"
#include <string>
Input::Input() {
	m_MousePos = glm::vec2(0);
}
Input::~Input() {

}

Input& Input::GetInstance() {
	static Input instance;
	return instance;
}

void Input::Update() {
	m_MouseDelta = glm::dvec2(0);
	memcpy(m_KeysLastFrame, m_Keys, sizeof(int) * GLFW_KEY_LAST);
	memcpy(m_MouseButtonsLastFrame, m_MouseButtons, sizeof(int) * GLFW_MOUSE_BUTTON_LAST);
}

void Input::SetKeyState(int key, int state){
	m_Keys[key] = state;
}

bool Input::IsKeyDown(int key){
	return m_Keys[key] == GLFW_PRESS || m_Keys[key] == GLFW_REPEAT;
}

bool Input::IsKeyPushed(int key) {
	return (m_KeysLastFrame[key] == GLFW_PRESS || m_KeysLastFrame[key] == GLFW_REPEAT) && m_Keys[key] == GLFW_RELEASE;
}

void Input::SetMousebuttonState(int button, int state) {
	m_MouseButtons[button] = state;
}
bool Input::IsMousebuttonDown(int button) {
	return m_MouseButtons[button] == GLFW_PRESS;
}

bool Input::IsMousebuttonPushed(int button) {
	return (m_MouseButtonsLastFrame[button] == GLFW_PRESS || m_MouseButtonsLastFrame[button] == GLFW_REPEAT) && m_MouseButtons[button] == GLFW_RELEASE;
}

void Input::SetCursorMode(GLFWwindow* window, int mode) {
	glfwSetInputMode(window, GLFW_CURSOR, mode);
	m_MouseDelta = glm::dvec2(0);
}

void Input::SetMousePos(double x, double y) {
	m_MouseDelta = glm::dvec2(x, y) - m_MousePos;
	m_MousePos = glm::dvec2(x, y);

}

glm::dvec2 Input::GetMouseDelta() {
	return m_MouseDelta;
}

glm::dvec2 Input::GetMousePos() {
	return m_MousePos;
}