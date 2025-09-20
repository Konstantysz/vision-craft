#include "ImGuiBeginLayer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Application.h"

namespace VisionCraft
{
    ImGuiBeginLayer::ImGuiBeginLayer()
    {
        // ImGui initialization will be done in InitializeImGui() on first render
    }

    ImGuiBeginLayer::~ImGuiBeginLayer()
    {
        if (initialized)
        {
            // TODO: Fix GLFW shutdown order - ImGui backends are calling GLFW functions after glfwTerminate()
            // This causes GLFW errors during application shutdown. Need to coordinate shutdown order
            // between Application and ImGui layers.
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void ImGuiBeginLayer::OnEvent(Core::Event& event)
    {
        if (blockEvents && initialized)
        {
            ImGuiIO& io = ImGui::GetIO();
            // Block events if ImGui wants to capture them
        }
    }

    void ImGuiBeginLayer::OnUpdate(float deltaTime)
    {
    }

    void ImGuiBeginLayer::OnRender()
    {
        if (!initialized)
        {
            InitializeImGui();
            initialized = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiBeginLayer::InitializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        GLFWwindow* window = glfwGetCurrentContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }
}