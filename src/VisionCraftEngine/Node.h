#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace VisionCraft::Engine
{
    /**
     * @brief Alias for a Node ID.
     */
    using NodeId = int;

    /**
     * @brief Structure representing a single parameter of a node.
     *
     * This structure holds the name and value of a parameter that can be attached to a node.
     */
    struct NodeParam
    {
        std::string name;  ///< Name of the parameter
        std::string value; ///< Value of the parameter
    };

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
        NodeId GetId() const;

        /**
         * @brief Get the name of the node.
         * @return const std::string& The node's name.
         */
        const std::string &GetName() const;

        /**
         * @brief Get the parameters of the node (const).
         * @return const std::vector<NodeParam>& Vector of node parameters.
         */

        const std::vector<NodeParam> &GetParams() const;

        /**
         * @brief Get the parameters of the node.
         * @return std::vector<NodeParam>& Vector of node parameters.
         */
        std::vector<NodeParam> &GetParams();

        /**
         * @brief Get the value of a parameter by name.
         * @param paramName Name of the parameter.
         * @return std::optional<std::string> Value if found, std::nullopt otherwise.
         */
        std::optional<std::string> GetParamValue(const std::string &paramName) const;

        /**
         * @brief Set the value of a parameter by name. Adds the parameter if it does not exist.
         * @param paramName Name of the parameter.
         * @param value Value to set.
         */
        void SetParamValue(const std::string &paramName, const std::string &value);

        /**
         * @brief Abstract method for processing node data. Must be implemented by derived classes.
         */
        virtual void Process() = 0;

    protected:
        /**
         * @brief Gets a parameter value as a double with validation.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist or is invalid
         * @param minValue Minimum allowed value (optional)
         * @param maxValue Maximum allowed value (optional)
         * @return Validated double value
         */
        double GetValidatedDoubleParam(const std::string& paramName, double defaultValue,
                                       std::optional<double> minValue = std::nullopt,
                                       std::optional<double> maxValue = std::nullopt) const;

        /**
         * @brief Gets a parameter value as an integer with validation.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist or is invalid
         * @param minValue Minimum allowed value (optional)
         * @param maxValue Maximum allowed value (optional)
         * @return Validated integer value
         */
        int GetValidatedIntParam(const std::string& paramName, int defaultValue,
                                 std::optional<int> minValue = std::nullopt,
                                 std::optional<int> maxValue = std::nullopt) const;

        /**
         * @brief Gets a parameter value as a boolean.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist or is invalid
         * @return Boolean value (true for "true", "1", "yes", false otherwise)
         */
        bool GetBoolParam(const std::string& paramName, bool defaultValue = false) const;

        /**
         * @brief Gets a parameter value as a string with validation.
         * @param paramName Name of the parameter
         * @param defaultValue Default value if parameter doesn't exist
         * @param allowedValues Optional set of allowed values for validation
         * @return Validated string value
         */
        std::string GetValidatedStringParam(const std::string& paramName,
                                            const std::string& defaultValue,
                                            const std::vector<std::string>& allowedValues = {}) const;

        /**
         * @brief Validates that a file path parameter exists and is accessible.
         * @param paramName Name of the parameter containing the file path
         * @param requireExistence Whether the file must exist (true) or just be a valid path (false)
         * @return True if path is valid, false otherwise
         */
        bool ValidateFilePath(const std::string& paramName, bool requireExistence = true) const;

        NodeId id;                     ///< Unique identifier of the node
        std::string name;              ///< Name of the node
        std::vector<NodeParam> params; ///< Parameters of the node
    };

    /**
     * @brief Alias for a unique pointer to a Node.
     */
    using NodePtr = std::unique_ptr<Node>;

} // namespace VisionCraft::Engine
