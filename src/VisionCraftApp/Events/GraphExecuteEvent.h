#pragma once

#include "Event.h"

namespace VisionCraft
{
    /**
     * @brief Event triggered when the node graph should be executed.
     *
     * This event is published when a user requests execution of the node graph
     * (e.g., by clicking an "Execute" button). The GraphExecutionLayer subscribes
     * to this event to trigger the actual graph processing.
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