#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "NodeParameter.h"
#include "Slot.h"


namespace VisionCraft::Engine
{
    /**
     * @brief Alias for a Node ID.
     */
    using NodeId = int;

    /**
     * @brief Abstract base class for all nodes in the editor.
     *
     * Node represents a single block in the dataflow graph. It stores its unique identifier, name, and parameters.
     * Derived classes should implement the Process() method to define node-specific processing logic.
     */
    class Node
    {
    public:
        /**
         * @brief Construct a new Node object.
         * @param id Unique identifier for the node.
         * @param name Name of the node.
         */
        Node(NodeId id, std::string name);

        /**
         * @brief Destroy the Node object.
         */
        virtual ~Node() = default;

        /**
         * @brief Get the unique identifier of the node.
         * @return NodeId The node's ID.
         */
        [[nodiscard]] NodeId GetId() const;

        /**
         * @brief Get the name of the node.
         * @return const std::string& The node's name.
         */
        [[nodiscard]] const std::string &GetName() const;

        /**
         * @brief Gets the modern parameter storage.
         * @return Reference to the parameter storage system
         */
        [[nodiscard]] ParameterStorage &GetParameters();

        /**
         * @brief Gets the modern parameter storage (const version).
         * @return Const reference to the parameter storage system
         */
        [[nodiscard]] const ParameterStorage &GetParameters() const;

        /**
         * @brief Sets a parameter value with automatic type deduction.
         * @tparam T Parameter value type
         * @param paramName Name of the parameter
         * @param value Value to set
         */
        template<typename T> void SetParam(const std::string &paramName, T &&value);

        /**
         * @brief Gets a parameter value with type checking.
         * @tparam T Expected parameter type
         * @param paramName Name of the parameter
         * @return Optional value if exists and correct type
         */
        template<typename T> [[nodiscard]] std::optional<T> GetParam(const std::string &paramName) const;

        /**
         * @brief Gets a parameter value with default fallback.
         * @tparam T Parameter type
         * @param paramName Name of the parameter
         * @param defaultValue Default if not found or wrong type
         * @return Parameter value or default
         */
        template<typename T> [[nodiscard]] T GetParamOr(const std::string &paramName, T defaultValue) const;

        /**
         * @brief Checks if parameter exists.
         * @param paramName Parameter name
         * @return True if parameter exists
         */
        [[nodiscard]] bool HasParameter(const std::string &paramName) const;

        /**
         * @brief Gets all parameter names.
         * @return Vector of parameter names
         */
        [[nodiscard]] std::vector<std::string> GetParameterNames() const;

        /**
         * @brief Gets a boolean parameter with smart parsing.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist or is invalid
         * @return Boolean value (supports: true/false, 1/0, yes/no, on/off)
         */
        [[nodiscard]] bool GetBoolParam(const std::string &paramName, bool defaultValue = false) const;

        /**
         * @brief Gets a filesystem path parameter with smart type handling.
         * @param paramName Name of the parameter
         * @param defaultPath Default path if parameter doesn't exist
         * @return Filesystem path parameter
         */
        [[nodiscard]] std::filesystem::path GetPath(const std::string &paramName,
            const std::filesystem::path &defaultPath = {}) const;

        /**
         * @brief Validates a file path parameter.
         * @param paramName Name of the parameter containing the file path
         * @param validation File path validation configuration
         * @return True if path is valid, false otherwise
         */
        [[nodiscard]] bool ValidateFilePath(const std::string &paramName,
            const FilePathValidation &validation = {}) const;

        /**
         * @brief Gets a validated string parameter.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist
         * @param validation String validation configuration
         * @return Validated string value
         */
        [[nodiscard]] std::string GetValidatedString(const std::string &paramName,
            const std::string &defaultValue,
            const StringValidation &validation = {}) const;

        /**
         * @brief Gets a validated numeric parameter with range checking.
         * @tparam T Numeric type (int or double)
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist or is invalid
         * @param validation Range validation configuration
         * @return Validated numeric value
         */
        template<NumericParameter T>
        [[nodiscard]] T GetValidatedParam(const std::string &paramName,
            T defaultValue,
            const ValidationRange<T> &validation = {}) const;

        /**
         * @brief Abstract method for processing node data. Must be implemented by derived classes.
         */
        virtual void Process() = 0;

        /**
         * @brief Calculates extra height needed for node-specific content.
         * Uses polymorphism instead of dynamic_cast chains.
         * @param nodeContentWidth Available content width
         * @param zoomLevel Current zoom level
         * @return Extra height required for custom content (0 if none)
         */
        [[nodiscard]] virtual float CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const;

        /**
         * @brief Creates input slot without default value.
         * @param slotName Name of the input slot
         * @return Reference to the created slot
         */
        Slot &CreateInputSlot(const std::string &slotName);

        /**
         * @brief Creates input slot with default value.
         * @tparam T Type of the default value
         * @param slotName Name of the input slot
         * @param defaultValue Default value when not connected
         * @return Reference to the created slot
         */
        template<typename T> Slot &CreateInputSlot(const std::string &slotName, T defaultValue);

        /**
         * @brief Creates output slot.
         * @param slotName Name of the output slot
         * @return Reference to the created slot
         */
        Slot &CreateOutputSlot(const std::string &slotName);

        /**
         * @brief Gets input slot.
         * @param slotName Name of the input slot
         * @return Const reference to the input slot
         */
        [[nodiscard]] const Slot &GetInputSlot(const std::string &slotName) const;

        /**
         * @brief Gets output slot.
         * @param slotName Name of the output slot
         * @return Const reference to the output slot
         */
        [[nodiscard]] const Slot &GetOutputSlot(const std::string &slotName) const;

        /**
         * @brief Sets data in input slot.
         * @param slotName Name of the input slot
         * @param data Data to set
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetInputSlotData(const std::string &slotName, NodeData data);

        /**
         * @brief Sets data in output slot.
         * @param slotName Name of the output slot
         * @param data Data to set
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetOutputSlotData(const std::string &slotName, NodeData data);

        /**
         * @brief Clears input slot data.
         * @param slotName Name of the input slot
         * @throws std::out_of_range if slot doesn't exist
         */
        void ClearInputSlot(const std::string &slotName);

        /**
         * @brief Clears output slot data.
         * @param slotName Name of the output slot
         * @throws std::out_of_range if slot doesn't exist
         */
        void ClearOutputSlot(const std::string &slotName);

        /**
         * @brief Gets input value with automatic fallback to default.
         * @tparam T Expected type
         * @param slotName Name of the input slot
         * @return Connected value if available, otherwise default value
         * @throws std::out_of_range if slot doesn't exist
         */
        template<typename T> [[nodiscard]] std::optional<T> GetInputValue(const std::string &slotName) const
        {
            return GetInputSlot(slotName).GetValueOrDefault<T>();
        }

        /**
         * @brief Sets default value for input slot.
         * @param slotName Name of the input slot
         * @param defaultValue New default value
         * @throws std::out_of_range if slot doesn't exist
         */
        void SetInputSlotDefault(const std::string &slotName, NodeData defaultValue);

        /**
         * @brief Checks if input slot is connected.
         * @param slotName Name of the slot
         * @return True if connected
         */
        [[nodiscard]] bool IsInputSlotConnected(const std::string &slotName) const;

        /**
         * @brief Checks if an input slot exists.
         * @param slotName Name of the slot to check
         * @return True if the input slot exists
         */
        [[nodiscard]] bool HasInputSlot(const std::string &slotName) const;

        /**
         * @brief Checks if an output slot exists.
         * @param slotName Name of the slot to check
         * @return True if the output slot exists
         */
        [[nodiscard]] bool HasOutputSlot(const std::string &slotName) const;

    protected:
        ParameterStorage parameters;                       ///< Modern type-safe parameter storage
        std::string name;                                  ///< Name of the node
        NodeId id;                                         ///< Unique identifier of the node
        std::unordered_map<std::string, Slot> inputSlots;  ///< Input data slots
        std::unordered_map<std::string, Slot> outputSlots; ///< Output data slots
    };

    // Node template method instantiation declarations
    extern template void Node::SetParam<int>(const std::string &, int &&);
    extern template void Node::SetParam<double>(const std::string &, double &&);
    extern template void Node::SetParam<bool>(const std::string &, bool &&);
    extern template void Node::SetParam<std::string>(const std::string &, std::string &&);
    extern template void Node::SetParam<std::filesystem::path>(const std::string &, std::filesystem::path &&);
    extern template void Node::SetParam<int &>(const std::string &, int &);
    extern template void Node::SetParam<double &>(const std::string &, double &);
    extern template void Node::SetParam<bool &>(const std::string &, bool &);
    extern template void Node::SetParam<std::string &>(const std::string &, std::string &);
    extern template void Node::SetParam<std::filesystem::path &>(const std::string &, std::filesystem::path &);

    extern template std::optional<int> Node::GetParam<int>(const std::string &) const;
    extern template std::optional<double> Node::GetParam<double>(const std::string &) const;
    extern template std::optional<bool> Node::GetParam<bool>(const std::string &) const;
    extern template std::optional<std::string> Node::GetParam<std::string>(const std::string &) const;
    extern template std::optional<std::filesystem::path> Node::GetParam<std::filesystem::path>(
        const std::string &) const;

    extern template int Node::GetParamOr<int>(const std::string &, int) const;
    extern template double Node::GetParamOr<double>(const std::string &, double) const;
    extern template bool Node::GetParamOr<bool>(const std::string &, bool) const;
    extern template std::string Node::GetParamOr<std::string>(const std::string &, std::string) const;
    extern template std::filesystem::path Node::GetParamOr<std::filesystem::path>(const std::string &,
        std::filesystem::path) const;

    extern template int Node::GetValidatedParam<int>(const std::string &, int, const ValidationRange<int> &) const;
    extern template double
        Node::GetValidatedParam<double>(const std::string &, double, const ValidationRange<double> &) const;

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
