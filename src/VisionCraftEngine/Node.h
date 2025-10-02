#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Slot.h"


namespace VisionCraft::Engine
{
    /**
     * @brief Alias for a Node ID.
     */
    using NodeId = int;

    /**
     * @brief Abstract base class for all nodes in the editor.
     */
    class Node
    {
    public:
        /**
         * @brief Constructs a node.
         * @param id Unique identifier
         * @param name Node name
         */
        Node(NodeId id, std::string name);

        /**
         * @brief Destroy the Node object.
         */
        virtual ~Node() = default;

        /**
         * @brief Returns the node's unique ID.
         * @return Node ID
         */
        [[nodiscard]] NodeId GetId() const;

        /**
         * @brief Returns the node's name.
         * @return Node name
         */
        [[nodiscard]] const std::string &GetName() const;

        /**
         * @brief Processes node data. Must be implemented by derived classes.
         */
        virtual void Process() = 0;

        /**
         * @brief Calculates extra height for node-specific content.
         * @param nodeContentWidth Available content width
         * @param zoomLevel Current zoom level
         * @return Extra height required (0 if none)
         */
        [[nodiscard]] virtual float CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const;

        /**
         * @brief Creates input slot without default value.
         * @param slotName Slot name
         * @return Created slot
         */
        Slot &CreateInputSlot(const std::string &slotName);

        /**
         * @brief Creates input slot with default value.
         * @param slotName Slot name
         * @param defaultValue Default value when not connected
         * @return Created slot
         */
        template<typename T> Slot &CreateInputSlot(const std::string &slotName, T defaultValue);

        /**
         * @brief Creates output slot.
         * @param slotName Slot name
         * @return Created slot
         */
        Slot &CreateOutputSlot(const std::string &slotName);

        /**
         * @brief Returns input slot.
         * @param slotName Slot name
         * @return Input slot
         */
        [[nodiscard]] const Slot &GetInputSlot(const std::string &slotName) const;

        /**
         * @brief Returns output slot.
         * @param slotName Slot name
         * @return Output slot
         */
        [[nodiscard]] const Slot &GetOutputSlot(const std::string &slotName) const;

        /**
         * @brief Sets data in input slot.
         * @param slotName Slot name
         * @param data Data to set
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetInputSlotData(const std::string &slotName, NodeData data);

        /**
         * @brief Sets data in output slot.
         * @param slotName Slot name
         * @param data Data to set
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetOutputSlotData(const std::string &slotName, NodeData data);

        /**
         * @brief Clears input slot data.
         * @param slotName Slot name
         * @throws std::out_of_range if slot doesn't exist
         */
        void ClearInputSlot(const std::string &slotName);

        /**
         * @brief Clears output slot data.
         * @param slotName Slot name
         * @throws std::out_of_range if slot doesn't exist
         */
        void ClearOutputSlot(const std::string &slotName);

        /**
         * @brief Returns input value with automatic fallback to default.
         * @param slotName Slot name
         * @return Connected value if available, otherwise default value
         * @throws std::out_of_range if slot doesn't exist
         */
        template<typename T> [[nodiscard]] std::optional<T> GetInputValue(const std::string &slotName) const
        {
            return GetInputSlot(slotName).GetValueOrDefault<T>();
        }

        /**
         * @brief Sets default value for input slot.
         * @param slotName Slot name
         * @param defaultValue New default value
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetInputSlotDefault(const std::string &slotName, NodeData defaultValue);

        /**
         * @brief Checks if input slot is connected.
         * @param slotName Slot name
         * @return True if connected
         */
        [[nodiscard]] bool IsInputSlotConnected(const std::string &slotName) const;

        /**
         * @brief Checks if input slot exists.
         * @param slotName Slot name
         * @return True if exists
         */
        [[nodiscard]] bool HasInputSlot(const std::string &slotName) const;

        /**
         * @brief Checks if output slot exists.
         * @param slotName Slot name
         * @return True if exists
         */
        [[nodiscard]] bool HasOutputSlot(const std::string &slotName) const;

    protected:
        std::string name;                                  ///< Name of the node
        NodeId id;                                         ///< Unique identifier of the node
        std::unordered_map<std::string, Slot> inputSlots;  ///< Input data slots
        std::unordered_map<std::string, Slot> outputSlots; ///< Output data slots
    };

    /**
     * @brief Alias for a unique pointer to a Node.
     */
    using NodePtr = std::unique_ptr<Node>;

    // Explicit instantiations for CreateInputSlot
    extern template Slot &Node::CreateInputSlot<double>(const std::string &, double);
    extern template Slot &Node::CreateInputSlot<float>(const std::string &, float);
    extern template Slot &Node::CreateInputSlot<int>(const std::string &, int);
    extern template Slot &Node::CreateInputSlot<bool>(const std::string &, bool);
    extern template Slot &Node::CreateInputSlot<std::string>(const std::string &, std::string);
    extern template Slot &Node::CreateInputSlot<std::filesystem::path>(const std::string &, std::filesystem::path);

} // namespace VisionCraft::Engine
