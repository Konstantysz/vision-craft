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
     * @brief Event bus for decoupled communication between application layers.
     *
     * EventBus implements a publish-subscribe pattern allowing layers to communicate
     * without direct dependencies. Layers can subscribe to specific event types and
     * publish events that other layers can respond to.
     *
     * Usage:
     * @code
     * // Subscribe to an event
     * eventBus.Subscribe<MyEvent>([](const MyEvent& event) {
     *     // Handle event
     * });
     *
     * // Publish an event
     * eventBus.Publish(MyEvent{data});
     * @endcode
     */
    class EventBus
    {
    public:
        /**
         * @brief Subscribes to events of a specific type.
         * @tparam TEvent The event type to subscribe to (must derive from Event)
         * @param callback Function to call when event is published
         *
         * The callback will be invoked whenever an event of type TEvent is published.
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
         * @tparam TEvent The event type (must derive from Event)
         * @param event The event to publish
         *
         * All callbacks subscribed to this event type will be invoked synchronously.
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
         *
         * Removes all event subscriptions. Useful for cleanup or reset.
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