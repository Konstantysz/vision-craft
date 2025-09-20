#pragma once

#include "Layer.h"

class VisionCraftAppLayer : public Core::Layer
{
public:
    VisionCraftAppLayer();

    virtual ~VisionCraftAppLayer();

    virtual OnUpdate(float deltaTime) override;

    virtual OnRender() override;
};