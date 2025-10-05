#pragma once

#include "Event.h"

namespace VisionCraft
{
    /**
     * @brief Event for triggering graph load.
     */
    class LoadGraphEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Default constructor.
         */
        LoadGraphEvent() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~LoadGraphEvent() = default;
    };
} // namespace VisionCraft