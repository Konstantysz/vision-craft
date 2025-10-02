#pragma once

#include "Event.h"

namespace VisionCraft
{
    /**
     * @brief Event for triggering graph execution.
     */
    class GraphExecuteEvent : public Core::Event
    {
    public:
        /**
         * @brief Default constructor.
         */
        GraphExecuteEvent() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~GraphExecuteEvent() = default;
    };
} // namespace VisionCraft