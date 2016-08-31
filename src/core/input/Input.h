#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define g_Input Input::GetInstance()
class Input {
public:
	~Input();
	static Input& GetInstance();
	void SetKeyState(int key, int state);
	void SetMousebuttonState(int button, int state);
	bool IsMousebuttonDown(int button);
	bool IsMousebuttonPushed(int button);
	bool IsKeyDown(int key);
	bool IsKeyPushed(int key);
	void SetCursorMode(GLFWwindow* window,int mode);
	void SetMousePos(double x, double y);
	glm::dvec2 GetMouseDelta();
	glm::dvec2 GetMousePos();
	void Update();
private:
	Input();
	int m_Keys[GLFW_KEY_LAST];
	int m_KeysLastFrame[GLFW_KEY_LAST];
	int m_MouseButtons[GLFW_MOUSE_BUTTON_LAST];
	int m_MouseButtonsLastFrame[GLFW_MOUSE_BUTTON_LAST];
	glm::dvec2 m_MousePos;
	glm::dvec2 m_MouseDelta;
};
static void MousePosCallback(GLFWwindow* window, double xpos, double ypos){
	g_Input.SetMousePos(xpos, ypos);
}
static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
	g_Input.SetMousebuttonState(button, action);
}
static void KeyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	g_Input.SetKeyState(key, action);
}