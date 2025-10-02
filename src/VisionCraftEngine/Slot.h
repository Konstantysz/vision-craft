#pragma once

#include "NodeData.h"
#include <optional>
#include <string>

namespace VisionCraft::Engine
{
    /**
     * @brief Type-safe data slot for node input/output connections.
     *
     * Slot uses std::variant for type-safe, zero-cost data storage.
     * Unlike the old dynamic_cast approach, this:
     * - Has zero heap allocation overhead
     * - Provides compile-time type safety
     * - Eliminates need for NodeEditor to know about concrete node types
     * - Supports pattern matching with std::visit
     *
     * Example usage:
     * @code
     * // Setting data (in Process())
     * GetOutputSlot("Output").SetData(processedImage);
     *
     * // Getting data (in Process())
     * auto input = GetInputSlot("Input").GetData<cv::Mat>();
     * if (input) {
     *     // Process the image...
     * }
     *
     * // Checking if slot has data
     * if (GetInputSlot("Input").HasData()) {
     *     // ...
     * }
     * @endcode
     */
    class Slot
    {
    public:
        /**
         * @brief Constructs an empty slot.
         */
        Slot() = default;

        /**
         * @brief Sets the data in this slot.
         * @param data The data to store (any type in NodeData variant)
         *
         * This will replace any existing data in the slot.
         * Pass by value for optimal move semantics with std::variant.
         */
        void SetData(NodeData data);

        /**
         * @brief Gets typed data from this slot.
         * @tparam T The expected type (must be one of the types in NodeData)
         * @return Optional containing the data if type matches, std::nullopt otherwise
         *
         * Example:
         * @code
         * auto image = slot.GetData<cv::Mat>();
         * if (image) {
         *     // Use *image
         * }
         * @endcode
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
         *
         * Useful for debugging or introspection.
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

    private:
        NodeData data; ///< The variant holding the actual data
    };

} // namespace VisionCraft::Engine