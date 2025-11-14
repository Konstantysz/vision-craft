#pragma once

#include "Event.h"

namespace VisionCraft::UI::Events
{
    /**
     * @brief Event for creating new graph (clearing current).
     */
    class NewGraphEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Default constructor.
         */
        NewGraphEvent() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~NewGraphEvent() = default;
    };
} // namespace VisionCraft::UI::Events
