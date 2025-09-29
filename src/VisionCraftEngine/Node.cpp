#include "Node.h"

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

    ParameterStorage &Node::GetParameters()
    {
        return parameters;
    }

    const ParameterStorage &Node::GetParameters() const
    {
        return parameters;
    }

    bool Node::HasParameter(const std::string &paramName) const
    {
        return parameters.HasParameter(paramName);
    }

    std::vector<std::string> Node::GetParameterNames() const
    {
        return parameters.GetParameterNames();
    }

    bool Node::GetBoolParam(const std::string &paramName, bool defaultValue) const
    {
        return parameters.GetBool(paramName, defaultValue);
    }

    std::filesystem::path Node::GetPath(const std::string &paramName, const std::filesystem::path &defaultPath) const
    {
        return parameters.GetPath(paramName, defaultPath);
    }

    bool Node::ValidateFilePath(const std::string &paramName, const FilePathValidation &validation) const
    {
        return parameters.ValidateFilePath(paramName, validation, GetName());
    }

    std::string Node::GetValidatedString(const std::string &paramName,
        const std::string &defaultValue,
        const StringValidation &validation) const
    {
        return parameters.GetValidatedString(paramName, defaultValue, validation, GetName());
    }

    template<typename T> void Node::SetParam(const std::string &paramName, T &&value)
    {
        parameters.Set(paramName, std::forward<T>(value));
    }

    template<typename T> std::optional<T> Node::GetParam(const std::string &paramName) const
    {
        return parameters.Get<T>(paramName);
    }

    template<typename T> T Node::GetParamOr(const std::string &paramName, T defaultValue) const
    {
        return parameters.GetOr(paramName, std::move(defaultValue));
    }

    template<NumericParameter T>
    T Node::GetValidatedParam(const std::string &paramName, T defaultValue, const ValidationRange<T> &validation) const
    {
        return parameters.GetValidated(paramName, defaultValue, validation, GetName());
    }

    // ================================================================
    // Explicit Template Instantiations
    // ================================================================
    template void Node::SetParam<int>(const std::string &, int &&);
    template void Node::SetParam<double>(const std::string &, double &&);
    template void Node::SetParam<bool>(const std::string &, bool &&);
    template void Node::SetParam<std::string>(const std::string &, std::string &&);
    template void Node::SetParam<std::filesystem::path>(const std::string &, std::filesystem::path &&);
    template void Node::SetParam<int &>(const std::string &, int &);
    template void Node::SetParam<double &>(const std::string &, double &);
    template void Node::SetParam<bool &>(const std::string &, bool &);
    template void Node::SetParam<std::string &>(const std::string &, std::string &);
    template void Node::SetParam<std::filesystem::path &>(const std::string &, std::filesystem::path &);

    template std::optional<int> Node::GetParam<int>(const std::string &) const;
    template std::optional<double> Node::GetParam<double>(const std::string &) const;
    template std::optional<bool> Node::GetParam<bool>(const std::string &) const;
    template std::optional<std::string> Node::GetParam<std::string>(const std::string &) const;
    template std::optional<std::filesystem::path> Node::GetParam<std::filesystem::path>(const std::string &) const;

    template int Node::GetParamOr<int>(const std::string &, int) const;
    template double Node::GetParamOr<double>(const std::string &, double) const;
    template bool Node::GetParamOr<bool>(const std::string &, bool) const;
    template std::string Node::GetParamOr<std::string>(const std::string &, std::string) const;
    template std::filesystem::path Node::GetParamOr<std::filesystem::path>(const std::string &,
        std::filesystem::path) const;

    template int Node::GetValidatedParam<int>(const std::string &, int, const ValidationRange<int> &) const;
    template double Node::GetValidatedParam<double>(const std::string &, double, const ValidationRange<double> &) const;


    float Node::CalculateExtraHeight(float nodeContentWidth, float zoomLevel) const
    {
        // Default implementation: no extra height needed
        // Derived classes can override this for custom content
        return 0.0f;
    }

} // namespace VisionCraft::Engine