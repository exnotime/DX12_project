#pragma once
#include <string>
struct GLFWwindow;
struct WindowSettings {
	int Width = 1600;
	int Height = 900;
	int X = -1;
	int Y = -1;
	bool Fullscreen = false;
	bool Resizeable = false;
	bool BorderLess = false;
	bool Vsync = false;
	std::string Title = "Untitled window";
};
#define g_Window Window::GetInstance()
class Window {
  public:
	static Window& GetInstance();
	~Window( );
	void Initialize( const WindowSettings& windowSettings );

	GLFWwindow* GetWindow( ) const;
	const WindowSettings& GetWindowSettings( ) const;
	void MakeCurrent( );

  private:
	Window();
	GLFWwindow* m_Window = nullptr;
	void* m_GLContext;
	WindowSettings m_WindowSettings;
	int ScreenWidth, ScreenHeight;
};
