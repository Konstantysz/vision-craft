#pragma once

#include "Nodes/Core/NodeEditor.h"
#include <imgui.h>
#include <string>

namespace VisionCraft::UI::Widgets
{
    /**
     * @brief Structure representing the visual position of a node in the editor.
     */
    struct NodePosition
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    /**
     * @brief Type of pin connection.
     *
     * Execution pins (white wires) control flow of execution, while data pins
     * (colored wires) transfer data between nodes. White wires define "what executes
     * next" and colored wires define "what data flows where".
     */
    enum class PinType
    {
        Execution, // White wire - controls execution flow
        Data       // Colored wire - transfers data between nodes
    };

    /**
     * @brief Enum for different data types in the node editor.
     */
    enum class PinDataType
    {
        Execution, // Execution flow - White (for execution pins)
        Image,     // cv::Mat - Green
        String,    // std::string - Magenta
        Float,     // double - Light Blue
        Int,       // int - Cyan
        Bool,      // bool - Red
        Path       // std::filesystem::path - Orange
    };

    /**
     * @brief Structure representing an input or output pin on a node.
     */
    struct NodePin
    {
        std::string name;
        PinType pinType;      // NEW: Execution or Data pin
        PinDataType dataType; // Only relevant for Data pins
        bool isInput;
    };

    /**
     * @brief Structure holding pins separated by type and direction.
     */
    struct SeparatedPins
    {
        std::vector<NodePin> executionInputPins;
        std::vector<NodePin> executionOutputPins;
        std::vector<NodePin> dataInputPins;
        std::vector<NodePin> dataOutputPins;
    };

    /**
     * @brief Helper to separate pins by type and direction.
     */
    inline SeparatedPins SeparatePinsByType(const std::vector<NodePin> &pins)
    {
        SeparatedPins result;
        for (const auto &pin : pins)
        {
            if (pin.pinType == PinType::Execution)
            {
                if (pin.isInput)
                    result.executionInputPins.push_back(pin);
                else
                    result.executionOutputPins.push_back(pin);
            }
            else
            {
                if (pin.isInput)
                    result.dataInputPins.push_back(pin);
                else
                    result.dataOutputPins.push_back(pin);
            }
        }
        return result;
    }

    /**
     * @brief Structure containing pre-calculated node dimensions.
     */
    struct NodeDimensions
    {
        ImVec2 size;
        size_t inputPinCount;
        size_t outputPinCount;
        size_t parameterPinCount;
    };

    /**
     * @brief Unique identifier for a pin on a node.
     *
     * Uses C++20 three-way comparison (spaceship operator) to automatically generate
     * all six comparison operators (==, !=, <, <=, >, >=) with correct semantics.
     */
    struct PinId
    {
        Nodes::NodeId nodeId;
        std::string pinName;

        // C++20 spaceship operator - automatically generates all comparison operators
        auto operator<=>(const PinId &other) const = default;
    };

    /**
     * @brief Connection between two pins.
     *
     * Uses C++20 three-way comparison for automatic operator generation.
     *
     * @note Each input pin can only have ONE connection, but output pins can have multiple.
     */
    struct NodeConnection
    {
        PinId outputPin; // Source pin (must be output)
        PinId inputPin;  // Target pin (must be input, can only have one connection)

        // C++20 spaceship operator - automatically generates all comparison operators
        auto operator<=>(const NodeConnection &other) const = default;
    };

    /**
     * @brief Connection creation state.
     */
    struct ConnectionState
    {
        bool isCreating = false;
        PinId startPin;
        ImVec2 endPosition;
    };

} // namespace VisionCraft::UI::Widgets
