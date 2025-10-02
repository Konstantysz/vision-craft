#pragma once

#include "Event.h"

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Core
{
    /**
     * @brief Event bus for publish-subscribe communication between layers.
     */
    class EventBus
    {
    public:
        /**
         * @brief Subscribes to events of a specific type.
         * @tparam TEvent Event type (must derive from Event)
         * @param callback Callback function
         */
        template<typename TEvent>
            requires std::is_base_of_v<Event, TEvent>
        void Subscribe(std::function<void(const TEvent &)> callback)
        {
            const auto typeIndex = std::type_index(typeid(TEvent));
            auto wrapper = [callback](const Event &event) { callback(static_cast<const TEvent &>(event)); };
            subscribers[typeIndex].push_back(wrapper);
        }

        /**
         * @brief Publishes an event to all subscribers.
         * @tparam TEvent Event type (must derive from Event)
         * @param event Event to publish
         */
        template<typename TEvent>
            requires std::is_base_of_v<Event, TEvent>
        void Publish(const TEvent &event)
        {
            const auto typeIndex = std::type_index(typeid(TEvent));
            if (const auto it = subscribers.find(typeIndex); it != subscribers.end())
            {
                for (const auto &callback : it->second)
                {
                    callback(event);
                }
            }
        }

        /**
         * @brief Clears all subscribers.
         */
        void Clear()
        {
            subscribers.clear();
        }

    private:
        using EventCallback = std::function<void(const Event &)>;
        std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
    };
} // namespace Core