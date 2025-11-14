#include "App/VisionCraftApplication.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "UI/Layers/DockSpaceLayer.h"
#include "UI/Layers/GraphExecutionLayer.h"
#include "UI/Layers/NodeEditorLayer.h"
#include "UI/Layers/PropertyPanelLayer.h"
#include "Logger.h"
#include "WindowStatePersistence.h"

namespace VisionCraft::App
{
    VisionCraftApplication::VisionCraftApplication(const Kappa::ApplicationSpecification &specification)
        : Kappa::Application(specification)
    {
        LOG_INFO("VisionCraftApplication: Starting initialization");

        Kappa::WindowStatePersistence::LoadAndApply(GetWindow(), "window_state.json");

        LOG_INFO("VisionCraftApplication: Pushing DockSpaceLayer");
        PushLayer<UI::Layers::DockSpaceLayer>();
        LOG_INFO("VisionCraftApplication: Pushing NodeEditorLayer");
        PushLayer<UI::Layers::NodeEditorLayer>(nodeEditor);
        LOG_INFO("VisionCraftApplication: Pushing PropertyPanelLayer");
        PushLayer<UI::Layers::PropertyPanelLayer>();
        LOG_INFO("VisionCraftApplication: Pushing GraphExecutionLayer");
        PushLayer<UI::Layers::GraphExecutionLayer>(nodeEditor);
        LOG_INFO("VisionCraftApplication: Initialization complete");
    }

    VisionCraftApplication::~VisionCraftApplication()
    {
        Kappa::WindowStatePersistence::CaptureAndSave(GetWindow(), "window_state.json");

        if (imguiInitialized)
        {
            ShutdownImGui();
        }
    }

    Nodes::NodeEditor &VisionCraftApplication::GetNodeEditor()
    {
        return nodeEditor;
    }

    const Nodes::NodeEditor &VisionCraftApplication::GetNodeEditor() const
    {
        return nodeEditor;
    }

    void VisionCraftApplication::BeginFrame()
    {
        if (!imguiInitialized)
        {
            LOG_INFO("VisionCraftApplication: Initializing ImGui");
            InitializeImGui();
            imguiInitialized = true;
            LOG_INFO("VisionCraftApplication: ImGui initialized successfully");
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
        LOG_INFO("InitializeImGui: Checking ImGui version");
        IMGUI_CHECKVERSION();
        LOG_INFO("InitializeImGui: Creating ImGui context");
        ImGui::CreateContext();
        LOG_INFO("InitializeImGui: Configuring ImGui IO");
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        LOG_INFO("InitializeImGui: Setting style");
        ImGui::StyleColorsDark();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        LOG_INFO("InitializeImGui: Getting GLFW window context");
        GLFWwindow *glfwWindow = glfwGetCurrentContext();
        if (!glfwWindow)
        {
            LOG_ERROR("InitializeImGui: GLFW window context is NULL!");
            return;
        }
        LOG_INFO("InitializeImGui: Initializing ImGui GLFW backend");
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        LOG_INFO("InitializeImGui: Initializing ImGui OpenGL3 backend");
        ImGui_ImplOpenGL3_Init("#version 410");
        LOG_INFO("InitializeImGui: Complete");
    }

    void VisionCraftApplication::ShutdownImGui()
    {
        // TODO: Fix GLFW shutdown order - ImGui backends are calling GLFW functions after glfwTerminate()
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace VisionCraft::App
