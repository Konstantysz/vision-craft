#pragma once

#include "Layer.h"
#include "Node.h"
#include <vector>
#include <memory>

namespace VisionCraft
{
    class NodeEditorLayer : public Core::Layer
    {
    public:
        NodeEditorLayer() = default;
        virtual ~NodeEditorLayer() = default;

        void OnEvent(Core::Event& event) override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

    private:
        std::vector<std::unique_ptr<Engine::Node>> nodes;
        Engine::Node* selectedNode = nullptr;
        bool isDragging = false;
        float dragOffsetX = 0.0f;
        float dragOffsetY = 0.0f;
    };
}