#include "NodeParameter.h"
#include "Logger.h"

#include <algorithm>
#include <ranges>

namespace VisionCraft::Engine
{
    template<NumericParameter T>
    constexpr ValidationRange<T>::ValidationRange(T minVal, T maxVal) : min(minVal), max(maxVal)
    {
    }

    template<NumericParameter T> constexpr ValidationRange<T>::ValidationRange(T minVal) : min(minVal)
    {
    }

    template<NumericParameter T>
    T ValidationRange<T>::ValidateAndClamp(T value, const std::string &parameterName, const std::string &nodeName) const
    {
        if (min.has_value() && value < min.value())
        {
            LOG_WARN("Node {}: Parameter '{}' value ({}) below minimum ({}), clamping",
                nodeName,
                parameterName,
                value,
                min.value());
            return min.value();
        }

        if (max.has_value() && value > max.value())
        {
            LOG_WARN("Node {}: Parameter '{}' value ({}) above maximum ({}), clamping",
                nodeName,
                parameterName,
                value,
                max.value());
            return max.value();
        }

        return value;
    }

    // Explicit template instantiation definitions
    template class ValidationRange<int>;
    template class ValidationRange<double>;

    template<typename T> void ParameterStorage::Set(const std::string &name, T &&value)
    {
        parameters[name] = std::forward<T>(value);
    }

    template<typename T> std::optional<T> ParameterStorage::Get(const std::string &name) const
    {
        auto it = parameters.find(name);
        if (it == parameters.end())
        {
            return std::nullopt;
        }

        if (auto *value = std::get_if<T>(&it->second))
        {
            return *value;
        }

        return std::nullopt;
    }

    template<typename T> T ParameterStorage::GetOr(const std::string &name, T defaultValue) const
    {
        auto value = Get<T>(name);
        return value.value_or(std::move(defaultValue));
    }

    template<NumericParameter T>
    T ParameterStorage::GetValidated(const std::string &name,
        T defaultValue,
        const ValidationRange<T> &validation,
        const std::string &nodeName) const
    {
        auto value = GetOr(name, defaultValue);
        return validation.ValidateAndClamp(value, name, nodeName);
    }

    // Explicit template instantiations for ParameterStorage methods
    template void ParameterStorage::Set<int>(const std::string &, int &&);
    template void ParameterStorage::Set<double>(const std::string &, double &&);
    template void ParameterStorage::Set<bool>(const std::string &, bool &&);
    template void ParameterStorage::Set<std::string>(const std::string &, std::string &&);
    template void ParameterStorage::Set<std::filesystem::path>(const std::string &, std::filesystem::path &&);
    template void ParameterStorage::Set<int &>(const std::string &, int &);
    template void ParameterStorage::Set<double &>(const std::string &, double &);
    template void ParameterStorage::Set<bool &>(const std::string &, bool &);
    template void ParameterStorage::Set<std::string &>(const std::string &, std::string &);
    template void ParameterStorage::Set<std::filesystem::path &>(const std::string &, std::filesystem::path &);

    template std::optional<int> ParameterStorage::Get<int>(const std::string &) const;
    template std::optional<double> ParameterStorage::Get<double>(const std::string &) const;
    template std::optional<bool> ParameterStorage::Get<bool>(const std::string &) const;
    template std::optional<std::string> ParameterStorage::Get<std::string>(const std::string &) const;
    template std::optional<std::filesystem::path> ParameterStorage::Get<std::filesystem::path>(
        const std::string &) const;

    template int ParameterStorage::GetOr<int>(const std::string &, int) const;
    template double ParameterStorage::GetOr<double>(const std::string &, double) const;
    template bool ParameterStorage::GetOr<bool>(const std::string &, bool) const;
    template std::string ParameterStorage::GetOr<std::string>(const std::string &, std::string) const;
    template std::filesystem::path ParameterStorage::GetOr<std::filesystem::path>(const std::string &,
        std::filesystem::path) const;

    template int ParameterStorage::GetValidated<int>(const std::string &,
        int,
        const ValidationRange<int> &,
        const std::string &) const;
    template double ParameterStorage::GetValidated<double>(const std::string &,
        double,
        const ValidationRange<double> &,
        const std::string &) const;

    StringValidation::StringValidation(std::initializer_list<std::string> values, bool caseSens)
        : allowedValues(values), caseSensitive(caseSens)
    {
    }

    std::string StringValidation::ValidateOrDefault(const std::string &value,
        const std::string &defaultValue,
        const std::string &parameterName,
        const std::string &nodeName) const
    {
        if (allowedValues.empty())
        {
            return value;
        }

        auto isMatch = [this, &value](const std::string &allowed) {
            if (caseSensitive)
            {
                return value == allowed;
            }
            else
            {
                return std::ranges::equal(
                    value, allowed, [](char a, char b) { return std::tolower(a) == std::tolower(b); });
            }
        };

        auto it = std::ranges::find_if(allowedValues, isMatch);
        if (it != allowedValues.end())
        {
            return value;
        }

        LOG_WARN("Node {}: Invalid value '{}' for parameter '{}', using default '{}'",
            nodeName,
            value,
            parameterName,
            defaultValue);
        return defaultValue;
    }

    FilePathValidation::FilePathValidation(bool exists, bool empty) : mustExist(exists), allowEmpty(empty)
    {
    }

    bool FilePathValidation::ValidatePath(const std::filesystem::path &path,
        const std::string &parameterName,
        const std::string &nodeName) const
    {
        if (path.empty())
        {
            if (allowEmpty)
            {
                return true;
            }
            LOG_WARN("Node {}: File path parameter '{}' is empty", nodeName, parameterName);
            return false;
        }

        try
        {
            if (!path.has_filename() && !allowEmpty)
            {
                LOG_WARN("Node {}: Invalid file path '{}' for parameter '{}'", nodeName, path.string(), parameterName);
                return false;
            }

            if (!allowedExtensions.empty())
            {
                const auto extension = path.extension().string();
                auto extensionMatches = [&extension](const std::string &allowed) {
                    return std::ranges::equal(
                        extension, allowed, [](char a, char b) { return std::tolower(a) == std::tolower(b); });
                };

                if (!std::ranges::any_of(allowedExtensions, extensionMatches))
                {
                    std::string extensionList;
                    for (const auto &ext : allowedExtensions)
                    {
                        if (!extensionList.empty())
                            extensionList += ", ";
                        extensionList += ext;
                    }
                    LOG_WARN("Node {}: File '{}' has invalid extension for parameter '{}'. Allowed: [{}]",
                        nodeName,
                        path.string(),
                        parameterName,
                        extensionList);
                    return false;
                }
            }

            if (mustExist && !std::filesystem::exists(path))
            {
                LOG_WARN(
                    "Node {}: File '{}' does not exist for parameter '{}'", nodeName, path.string(), parameterName);
                return false;
            }

            return true;
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            LOG_ERROR("Node {}: Filesystem error validating path '{}' for parameter '{}': {}",
                nodeName,
                path.string(),
                parameterName,
                e.what());
            return false;
        }
    }

    std::string ParameterStorage::GetValidatedString(const std::string &name,
        const std::string &defaultValue,
        const StringValidation &validation,
        const std::string &nodeName) const
    {
        const auto value = GetOr(name, defaultValue);
        return validation.ValidateOrDefault(value, defaultValue, name, nodeName);
    }

    bool ParameterStorage::GetBool(const std::string &name, bool defaultValue) const
    {
        if (auto boolValue = Get<bool>(name))
        {
            return boolValue.value();
        }

        auto stringValue = Get<std::string>(name);
        if (!stringValue.has_value())
        {
            return defaultValue;
        }

        std::string lowerValue;
        std::ranges::transform(stringValue.value(), std::back_inserter(lowerValue), [](char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (std::ranges::any_of(
                ParameterConstants::kTrueValues, [&lowerValue](const char *trueVal) { return lowerValue == trueVal; }))
        {
            return true;
        }

        if (std::ranges::any_of(ParameterConstants::kFalseValues,
                [&lowerValue](const char *falseVal) { return lowerValue == falseVal; }))
        {
            return false;
        }

        return defaultValue;
    }

    bool ParameterStorage::ValidateFilePath(const std::string &name,
        const FilePathValidation &validation,
        const std::string &nodeName) const
    {
        const auto path = GetPath(name);
        return validation.ValidatePath(path, name, nodeName);
    }

    std::filesystem::path ParameterStorage::GetPath(const std::string &name,
        const std::filesystem::path &defaultPath) const
    {
        if (auto pathValue = Get<std::filesystem::path>(name))
        {
            return pathValue.value();
        }

        if (auto stringValue = Get<std::string>(name))
        {
            return std::filesystem::path(stringValue.value());
        }

        return defaultPath;
    }

    bool ParameterStorage::HasParameter(const std::string &name) const
    {
        return parameters.contains(name);
    }

    std::vector<std::string> ParameterStorage::GetParameterNames() const
    {
        std::vector<std::string> names;
        names.reserve(parameters.size());

        std::ranges::transform(parameters, std::back_inserter(names), [](const auto &pair) { return pair.first; });

        std::ranges::sort(names);

        return names;
    }

    void ParameterStorage::Clear()
    {
        parameters.clear();
    }

    size_t ParameterStorage::Count() const
    {
        return parameters.size();
    }

} // namespace VisionCraft::Engine