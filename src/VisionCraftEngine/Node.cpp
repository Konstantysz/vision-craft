#include "Node.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>

#include "Logger.h"

namespace VisionCraft::Engine
{

    Node::Node(NodeId id, std::string name) : id(id), name(std::move(name))
    {
    }

    NodeId Node::GetId() const
    {
        return id;
    }

    const std::string &Node::GetName() const
    {
        return name;
    }

    const std::vector<NodeParam> &Node::GetParams() const
    {
        return params;
    }

    std::vector<NodeParam> &Node::GetParams()
    {
        return params;
    }

    std::optional<std::string> Node::GetParamValue(const std::string &paramName) const
    {
        for (const auto &param : params)
        {
            if (param.name == paramName)
            {
                return param.value;
            }
        }

        return std::nullopt;
    }

    void Node::SetParamValue(const std::string &paramName, const std::string &value)
    {
        for (auto &param : params)
        {
            if (param.name == paramName)
            {
                param.value = value;
                return;
            }
        }

        params.push_back({ paramName, value });
    }

    double Node::GetValidatedDoubleParam(const std::string& paramName, double defaultValue,
                                         std::optional<double> minValue, std::optional<double> maxValue) const
    {
        auto paramValue = GetParamValue(paramName);
        if (!paramValue.has_value())
        {
            return defaultValue;
        }

        try
        {
            double value = std::stod(paramValue.value());

            if (minValue.has_value() && value < minValue.value())
            {
                LOG_WARN("Node {}: Parameter '{}' value ({}) below minimum ({}), clamping",
                         GetName(), paramName, value, minValue.value());
                value = minValue.value();
            }

            if (maxValue.has_value() && value > maxValue.value())
            {
                LOG_WARN("Node {}: Parameter '{}' value ({}) above maximum ({}), clamping",
                         GetName(), paramName, value, maxValue.value());
                value = maxValue.value();
            }

            return value;
        }
        catch (const std::exception& e)
        {
            LOG_WARN("Node {}: Invalid double value '{}' for parameter '{}', using default ({})",
                     GetName(), paramValue.value(), paramName, defaultValue);
            return defaultValue;
        }
    }

    int Node::GetValidatedIntParam(const std::string& paramName, int defaultValue,
                                   std::optional<int> minValue, std::optional<int> maxValue) const
    {
        auto paramValue = GetParamValue(paramName);
        if (!paramValue.has_value())
        {
            return defaultValue;
        }

        try
        {
            int value = std::stoi(paramValue.value());

            if (minValue.has_value() && value < minValue.value())
            {
                LOG_WARN("Node {}: Parameter '{}' value ({}) below minimum ({}), clamping",
                         GetName(), paramName, value, minValue.value());
                value = minValue.value();
            }

            if (maxValue.has_value() && value > maxValue.value())
            {
                LOG_WARN("Node {}: Parameter '{}' value ({}) above maximum ({}), clamping",
                         GetName(), paramName, value, maxValue.value());
                value = maxValue.value();
            }

            return value;
        }
        catch (const std::exception& e)
        {
            LOG_WARN("Node {}: Invalid integer value '{}' for parameter '{}', using default ({})",
                     GetName(), paramValue.value(), paramName, defaultValue);
            return defaultValue;
        }
    }

    bool Node::GetBoolParam(const std::string& paramName, bool defaultValue) const
    {
        auto paramValue = GetParamValue(paramName);
        if (!paramValue.has_value())
        {
            return defaultValue;
        }

        std::string value = paramValue.value();
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        return (value == "true" || value == "1" || value == "yes" || value == "on");
    }

    std::string Node::GetValidatedStringParam(const std::string& paramName,
                                              const std::string& defaultValue,
                                              const std::vector<std::string>& allowedValues) const
    {
        auto paramValue = GetParamValue(paramName);
        if (!paramValue.has_value())
        {
            return defaultValue;
        }

        const std::string& value = paramValue.value();
        if (allowedValues.empty())
        {
            return value;
        }

        auto it = std::find(allowedValues.begin(), allowedValues.end(), value);
        if (it != allowedValues.end())
        {
            return value;
        }

        LOG_WARN("Node {}: Invalid value '{}' for parameter '{}', using default '{}'",
                 GetName(), value, paramName, defaultValue);
        return defaultValue;
    }

    bool Node::ValidateFilePath(const std::string& paramName, bool requireExistence) const
    {
        auto paramValue = GetParamValue(paramName);
        if (!paramValue.has_value() || paramValue->empty())
        {
            LOG_WARN("Node {}: File path parameter '{}' is empty", GetName(), paramName);
            return false;
        }

        const std::string& filePath = paramValue.value();

        try
        {
            std::filesystem::path path(filePath);
            if (path.empty())
            {
                LOG_WARN("Node {}: Invalid file path '{}' for parameter '{}'", GetName(), filePath, paramName);
                return false;
            }

            if (requireExistence && !std::filesystem::exists(path))
            {
                LOG_WARN("Node {}: File '{}' does not exist for parameter '{}'", GetName(), filePath, paramName);
                return false;
            }

            return true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            LOG_ERROR("Node {}: Filesystem error validating path '{}' for parameter '{}': {}",
                      GetName(), filePath, paramName, e.what());
            return false;
        }
    }

} // namespace VisionCraft::Engine
