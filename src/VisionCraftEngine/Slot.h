#pragma once

#include "NodeData.h"
#include <optional>
#include <string>

namespace VisionCraft::Engine
{
    /**
     * @brief Type-safe data slot with optional default values.
     *
     * Connected data overrides defaults. Zero-cost std::variant storage.
     */
    class Slot
    {
    public:
        /**
         * @brief Default constructor for slots without defaults.
         */
        Slot() = default;

        /**
         * @brief Constructs a slot with a default value.
         * @param defaultValue Default value used when slot is not connected
         */
        explicit Slot(std::optional<NodeData> defaultValue);

        /**
         * @brief Sets the data in this slot.
         * @param data The data to store (any type in NodeData variant)
         */
        void SetData(NodeData data);

        /**
         * @brief Gets typed data from this slot.
         * @tparam T The expected type (must be one of the types in NodeData)
         * @return Optional containing the data if type matches, std::nullopt otherwise
         */
        template<typename T> [[nodiscard]] std::optional<T> GetData() const
        {
            if (std::holds_alternative<T>(data))
            {
                return std::get<T>(data);
            }
            return std::nullopt;
        }

        /**
         * @brief Checks if this slot contains any data.
         * @return True if slot has data, false if empty (std::monostate)
         */
        [[nodiscard]] bool HasData() const;

        /**
         * @brief Clears the data in this slot, resetting it to empty state.
         */
        void Clear();

        /**
         * @brief Gets the type index of the currently stored data.
         * @return Index into the variant (0 = monostate/empty, 1 = cv::Mat, etc.)
         */
        [[nodiscard]] size_t GetTypeIndex() const;

        /**
         * @brief Checks if this slot contains a specific type.
         * @tparam T Type to check for
         * @return True if slot contains data of type T
         */
        template<typename T> [[nodiscard]] bool HoldsType() const
        {
            return std::holds_alternative<T>(data);
        }

        /**
         * @brief Sets the default value for this slot (used when not connected).
         * @param defaultValue The default value to use
         */
        void SetDefaultValue(NodeData defaultValue);

        /**
         * @brief Gets the default value for this slot.
         * @tparam T The expected type of the default value
         * @return Optional containing the default value if type matches
         */
        template<typename T> [[nodiscard]] std::optional<T> GetDefaultValue() const
        {
            if (defaultValue && std::holds_alternative<T>(*defaultValue))
            {
                return std::get<T>(*defaultValue);
            }
            return std::nullopt;
        }

        /**
         * @brief Checks if this slot has a default value.
         * @return True if a default value is set
         */
        [[nodiscard]] bool HasDefaultValue() const;

        /**
         * @brief Gets value with automatic fallback to default.
         * @tparam T The expected type
         * @return Connected data if available, otherwise default value
         */
        template<typename T> [[nodiscard]] std::optional<T> GetValueOrDefault() const
        {
            if (HasData())
            {
                auto connectedData = GetData<T>();
                if (connectedData)
                    return connectedData;
            }

            return GetDefaultValue<T>();
        }

        /**
         * @brief Checks if this slot is currently connected (has runtime data).
         * @return True if slot has connected data, false if using defaults
         */
        [[nodiscard]] bool IsConnected() const;

        /**
         * @brief Gets raw variant data for data passing.
         * @return The NodeData variant
         */
        [[nodiscard]] const NodeData &GetVariantData() const;

    private:
        NodeData data;                        ///< Runtime data from connected node
        std::optional<NodeData> defaultValue; ///< UI-editable default value
    };

} // namespace VisionCraft::Engine