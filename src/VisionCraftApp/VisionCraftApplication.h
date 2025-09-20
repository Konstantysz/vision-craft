#pragma once

#include "Application.h"

namespace VisionCraft
{
    class VisionCraftApplication : public Core::Application
    {
    public:
        VisionCraftApplication(const Core::ApplicationSpecification &specification = Core::ApplicationSpecification());
        
        virtual ~VisionCraftApplication();

    protected:
        void BeginFrame() override;

        void EndFrame() override;

    private:
        void InitializeImGui();

        void ShutdownImGui();
        
        bool imguiInitialized = false;
    };
} // namespace VisionCraft