#pragma once

#include "NodeEditor.h"
#include <imgui.h>
#include <string>

namespace VisionCraft
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
     * @brief Enum for different data types in the node editor.
     */
    enum class PinDataType
    {
        Image,  // cv::Mat - Green
        String, // std::string - Magenta
        Float,  // double - Light Blue
        Int,    // int - Cyan
        Bool,   // bool - Red
        Path    // std::filesystem::path - Orange
    };

    /**
     * @brief Structure representing an input or output pin on a node.
     */
    struct NodePin
    {
        std::string name;
        PinDataType dataType;
        bool isInput;
    };

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
     */
    struct PinId
    {
        Engine::NodeId nodeId;
        std::string pinName;

        bool operator==(const PinId &other) const
        {
            return nodeId == other.nodeId && pinName == other.pinName;
        }

        bool operator<(const PinId &other) const
        {
            if (nodeId != other.nodeId)
                return nodeId < other.nodeId;
            return pinName < other.pinName;
        }
    };

    /**
     * @brief Represents a connection between two pins.
     * Note: Each input pin can only have ONE connection, but output pins can have multiple connections.
     */
    struct NodeConnection
    {
        PinId outputPin; // Source pin (must be output)
        PinId inputPin;  // Target pin (must be input, can only have one connection)

        bool operator==(const NodeConnection &other) const
        {
            return outputPin == other.outputPin && inputPin == other.inputPin;
        }
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

} // namespace VisionCraft