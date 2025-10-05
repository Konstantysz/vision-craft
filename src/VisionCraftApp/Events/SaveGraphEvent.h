#pragma once

#include "Event.h"

namespace VisionCraft
{
    /**
     * @brief Event for triggering graph save.
     */
    class SaveGraphEvent : public Kappa::Event
    {
    public:
        /**
         * @brief Default constructor.
         */
        SaveGraphEvent() = default;

        /**
         * @brief Virtual destructor.
         */
        virtual ~SaveGraphEvent() = default;
    };
} // namespace VisionCraft