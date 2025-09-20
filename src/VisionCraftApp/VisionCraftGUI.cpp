#include "VisionCraftApplication.h"

#include "Layers/DockSpaceLayer.h"
#include "Layers/CanvasLayer.h"
#include "Layers/NodeEditorLayer.h"
#include "Layers/PropertyPanelLayer.h"
#include "Layers/GraphExecutionLayer.h"

int main()
{
    // Create application specification
    Core::ApplicationSpecification appSpec;
    appSpec.name = "VisionCraft";
    appSpec.windowSpecification.title = "VisionCraft - Computer Vision Node Editor";
    appSpec.windowSpecification.width = 1920;
    appSpec.windowSpecification.height = 1080;

    // Create VisionCraft application with ImGui integration
    VisionCraft::VisionCraftApplication app(appSpec);

    // Add layers (no need for ImGuiBeginLayer/ImGuiEndLayer anymore)
    app.PushLayer<VisionCraft::DockSpaceLayer>();
    app.PushLayer<VisionCraft::CanvasLayer>();
    app.PushLayer<VisionCraft::NodeEditorLayer>();
    app.PushLayer<VisionCraft::PropertyPanelLayer>();
    app.PushLayer<VisionCraft::GraphExecutionLayer>();

    // Run the application
    app.Run();

    return 0;
}