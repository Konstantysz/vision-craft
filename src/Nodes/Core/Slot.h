#pragma once

#include "Nodes/Core/NodeData.h"
#include <optional>
#include <string>

namespace VisionCraft::Nodes
{
    /**
     * @brief Type-safe data slot with optional default values.
     */
    class Slot
    {
    public:
        /**
         * @brief Default constructor for slots without defaults.
         */
        Slot() = default;

        /**
         * @brief Constructs slot with default value.
         * @param defaultValue Default value when not connected
         */
        explicit Slot(std::optional<NodeData> defaultValue);

        /**
         * @brief Sets data in slot.
         * @param data Data to store
         */
        void SetData(NodeData data);

        /**
         * @brief Returns typed data from slot.
         * @return Data if type matches, std::nullopt otherwise
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
         * @brief Checks if slot contains data.
         * @return True if slot has data
         */
        [[nodiscard]] bool HasData() const;

        /**
         * @brief Clears slot data.
         */
        void Clear();

        /**
         * @brief Returns type index of stored data.
         * @return Variant index (0 = empty)
         */
        [[nodiscard]] size_t GetTypeIndex() const;

        /**
         * @brief Checks if slot contains specific type.
         * @return True if slot contains type T
         */
        template<typename T> [[nodiscard]] bool HoldsType() const
        {
            return std::holds_alternative<T>(data);
        }

        /**
         * @brief Sets default value for slot.
         * @param defaultValue Default value
         */
        void SetDefaultValue(NodeData defaultValue);

        /**
         * @brief Returns default value for slot.
         * @return Default value if type matches
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
         * @brief Checks if slot has default value.
         * @return True if default value is set
         */
        [[nodiscard]] bool HasDefaultValue() const;

        /**
         * @brief Returns value with automatic fallback to default.
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
         * @brief Checks if slot is connected.
         * @return True if slot has connected data
         */
        [[nodiscard]] bool IsConnected() const;

        /**
         * @brief Returns raw variant data.
         * @return NodeData variant
         */
        [[nodiscard]] const NodeData &GetVariantData() const;

    private:
        NodeData data;                        ///< Runtime data from connected node
        std::optional<NodeData> defaultValue; ///< UI-editable default value
    };

} // namespace VisionCraft::Nodes
