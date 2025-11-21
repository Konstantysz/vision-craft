#include "App/VisionCraftApplication.h"
#include "Vision/Factory/NodeFactory.h"

int main()
{
    // Register all node types before creating application
    VisionCraft::Vision::NodeFactory::RegisterAllNodes();

    Kappa::ApplicationSpecification appSpec;
    appSpec.name = "VisionCraft";
    appSpec.windowSpecification.title = "VisionCraft - Computer Vision Node Editor";
    appSpec.windowSpecification.width = 1920;
    appSpec.windowSpecification.height = 1080;

    VisionCraft::App::VisionCraftApplication app(appSpec);
    app.Run();

    return 0;
}
