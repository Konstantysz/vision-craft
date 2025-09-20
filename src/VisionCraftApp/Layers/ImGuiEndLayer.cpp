#include "ImGuiEndLayer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Application.h"

namespace VisionCraft
{
    void ImGuiEndLayer::OnEvent(Core::Event& event)
    {
    }

    void ImGuiEndLayer::OnUpdate(float deltaTime)
    {
    }

    void ImGuiEndLayer::OnRender()
    {
        ImGuiIO& io = ImGui::GetIO();
        auto& app = Core::Application::Get();
        auto framebufferSize = app.GetFramebufferSize();
        io.DisplaySize = ImVec2(framebufferSize.x, framebufferSize.y);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}