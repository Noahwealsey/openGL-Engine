#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#define BREAKER(x) if(!x) __debugbreak();
#define BREAKCALL(x) ClearError();\
    x;\
    BREAKER(LogCall(#x, __FILE__, __LINE__));

class Renderer {
public:
    static bool Initialize();
    static void Cleanup();
    static GLFWwindow* GetWindow() { return s_window; }
    static void InitializeImGui();
    static void BeginImGuiFrame();
    static void EndImGuiFrame();
    
    // Error handling
    static void ClearError();
    static bool LogCall(const char* function, const char* file, int line);

private:
    static GLFWwindow* s_window;
    static const int s_windowWidth = 1920;
    static const int s_windowHeight = 1080;
};