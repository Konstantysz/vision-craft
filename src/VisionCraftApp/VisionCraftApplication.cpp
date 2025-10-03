#include "VisionCraftApplication.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Layers/DockSpaceLayer.h"
#include "Layers/GraphExecutionLayer.h"
#include "Layers/NodeEditorLayer.h"
#include "Layers/PropertyPanelLayer.h"

namespace VisionCraft
{
    VisionCraftApplication::VisionCraftApplication(const Kappa::ApplicationSpecification &specification)
        : Kappa::Application(specification)
    {
        PushLayer<DockSpaceLayer>();
        PushLayer<NodeEditorLayer>();
        PushLayer<PropertyPanelLayer>();
        PushLayer<GraphExecutionLayer>();
    }

    VisionCraftApplication::~VisionCraftApplication()
    {
        if (imguiInitialized)
        {
            ShutdownImGui();
        }
    }

    Engine::NodeEditor &VisionCraftApplication::GetNodeEditor()
    {
        return nodeEditor;
    }

    const Engine::NodeEditor &VisionCraftApplication::GetNodeEditor() const
    {
        return nodeEditor;
    }

    void VisionCraftApplication::BeginFrame()
    {
        if (!imguiInitialized)
        {
            InitializeImGui();
            imguiInitialized = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VisionCraftApplication::EndFrame()
    {
        if (imguiInitialized)
        {
            ImGuiIO &io = ImGui::GetIO();
            auto framebufferSize = GetFramebufferSize();
            io.DisplaySize = ImVec2(framebufferSize.x, framebufferSize.y);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow *backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
        }
    }

    void VisionCraftApplication::InitializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        GLFWwindow *glfwWindow = glfwGetCurrentContext();
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void VisionCraftApplication::ShutdownImGui()
    {
        // TODO: Fix GLFW shutdown order - ImGui backends are calling GLFW functions after glfwTerminate()
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace VisionCraft