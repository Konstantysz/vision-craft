#include "App/VisionCraftApplication.h"

int main()
{
    Kappa::ApplicationSpecification appSpec;
    appSpec.name = "VisionCraft";
    appSpec.windowSpecification.title = "VisionCraft - Computer Vision Node Editor";
    appSpec.windowSpecification.width = 1920;
    appSpec.windowSpecification.height = 1080;

    VisionCraft::App::VisionCraftApplication app(appSpec);
    app.Run();

    return 0;
}
