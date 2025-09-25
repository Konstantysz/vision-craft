#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace VisionCraft::Engine
{
    /**
     * @brief Type-safe parameter value using modern C++ variant.
     *
     * Stores parameters with proper types instead of everything as strings.
     * Eliminates runtime parsing overhead and provides compile-time type safety.
     */
    using ParameterValue = std::variant<int, double, bool, std::string, std::filesystem::path>;

    /**
     * @brief Concept for numeric parameter types.
     */
    template<typename T>
    concept NumericParameter = std::same_as<T, int> || std::same_as<T, double>;

    /**
     * @brief Constants for boolean parameter parsing.
     */
    namespace ParameterConstants
    {
        constexpr std::array<const char *, 4> kTrueValues = { "true", "1", "yes", "on" };
        constexpr std::array<const char *, 4> kFalseValues = { "false", "0", "no", "off" };
    } // namespace ParameterConstants

    /**
     * @brief Modern parameter validation configuration.
     */
    template<NumericParameter T> struct ValidationRange
    {
        /** @brief Default constructor - no validation limits */
        constexpr ValidationRange() = default;

        /** @brief Constructor with both min and max limits
         * @param minVal Minimum allowed value
         * @param maxVal Maximum allowed value
         */
        constexpr ValidationRange(T minVal, T maxVal);

        /** @brief Constructor with only minimum limit
         * @param minVal Minimum allowed value
         */
        constexpr ValidationRange(T minVal);

        /**
         * @brief Validates and clamps a value within the range.
         * @param value Value to validate
         * @param parameterName Name for logging
         * @param nodeName Node name for logging
         * @return Clamped value within range
         */
        T ValidateAndClamp(T value, const std::string &parameterName, const std::string &nodeName) const;

        std::optional<T> min; ///< Minimum allowed value (optional)
        std::optional<T> max; ///< Maximum allowed value (optional)
    };

    // Explicit template instantiation declarations
    extern template struct ValidationRange<int>;
    extern template struct ValidationRange<double>;

    /**
     * @brief String validation configuration.
     */
    struct StringValidation
    {
        /** @brief Default constructor - no validation */
        constexpr StringValidation() = default;

        /** @brief Constructor with allowed values list
         * @param values List of allowed string values
         * @param caseSens Whether validation should be case-sensitive
         */
        StringValidation(std::initializer_list<std::string> values, bool caseSens = true);

        /**
         * @brief Validates a string value against allowed values.
         * @param value Value to validate
         * @param defaultValue Default if validation fails
         * @param parameterName Name for logging
         * @param nodeName Node name for logging
         * @return Valid string value
         */
        std::string ValidateOrDefault(const std::string &value,
            const std::string &defaultValue,
            const std::string &parameterName,
            const std::string &nodeName) const;

        std::vector<std::string> allowedValues; ///< List of allowed string values
        bool caseSensitive = true;              ///< Whether validation is case-sensitive
    };

    /**
     * @brief File path validation configuration.
     */
    struct FilePathValidation
    {
        /** @brief Default constructor - file must exist, empty paths not allowed */
        constexpr FilePathValidation() = default;

        /** @brief Constructor with existence and empty path options
         * @param exists Whether the file must exist on disk
         * @param empty Whether empty paths are allowed
         */
        FilePathValidation(bool exists, bool empty = false);

        /**
         * @brief Validates a file path.
         * @param path Path to validate
         * @param parameterName Name for logging
         * @param nodeName Node name for logging
         * @return True if path is valid
         */
        bool ValidatePath(const std::filesystem::path &path,
            const std::string &parameterName,
            const std::string &nodeName) const;

        std::vector<std::string> allowedExtensions; ///< List of allowed file extensions
        bool mustExist = true;                      ///< Whether the file must exist on disk
        bool allowEmpty = false;                    ///< Whether empty paths are allowed
    };

    /**
     * @brief Modern parameter storage with O(1) lookup and type safety.
     *
     * Replaces the old vector<NodeParam> with unordered_map for performance
     * and variant for type safety. Eliminates string parsing overhead.
     */
    class ParameterStorage
    {
    public:
        /**
         * @brief Sets a parameter value with automatic type deduction.
         * @tparam T Parameter value type
         * @param name Parameter name
         * @param value Parameter value
         */
        template<typename T> void Set(const std::string &name, T &&value);

        /**
         * @brief Gets a parameter value with type checking.
         * @tparam T Expected parameter type
         * @param name Parameter name
         * @return Optional value if exists and correct type
         */
        template<typename T> std::optional<T> Get(const std::string &name) const;

        /**
         * @brief Gets a parameter value with default fallback.
         * @tparam T Parameter type
         * @param name Parameter name
         * @param defaultValue Default if not found or wrong type
         * @return Parameter value or default
         */
        template<typename T> T GetOr(const std::string &name, T defaultValue) const;

        /**
         * @brief Gets validated numeric parameter with range checking.
         * @tparam T Numeric type (int or double)
         * @param name Parameter name
         * @param defaultValue Default value
         * @param validation Range validation config
         * @param nodeName Node name for logging
         * @return Validated parameter value
         */
        template<NumericParameter T>
        T GetValidated(const std::string &name,
            T defaultValue,
            const ValidationRange<T> &validation,
            const std::string &nodeName) const;

        /**
         * @brief Gets validated string parameter.
         * @param name Parameter name
         * @param defaultValue Default value
         * @param validation String validation config
         * @param nodeName Node name for logging
         * @return Validated string value
         */
        std::string GetValidatedString(const std::string &name,
            const std::string &defaultValue,
            const StringValidation &validation,
            const std::string &nodeName) const;

        /**
         * @brief Gets boolean parameter with smart parsing.
         * @param name Parameter name
         * @param defaultValue Default value
         * @return Boolean parameter value
         */
        bool GetBool(const std::string &name, bool defaultValue = false) const;

        /**
         * @brief Validates file path parameter.
         * @param name Parameter name
         * @param validation File path validation config
         * @param nodeName Node name for logging
         * @return True if path is valid
         */
        bool ValidateFilePath(const std::string &name,
            const FilePathValidation &validation,
            const std::string &nodeName) const;

        /**
         * @brief Gets a filesystem path parameter with smart type handling.
         * @param name Parameter name
         * @param defaultPath Default path if parameter doesn't exist
         * @return Filesystem path parameter
         */
        std::filesystem::path GetPath(const std::string &name, const std::filesystem::path &defaultPath = {}) const;

        /**
         * @brief Checks if parameter exists.
         * @param name Parameter name
         * @return True if parameter exists
         */
        bool HasParameter(const std::string &name) const;

        /**
         * @brief Gets all parameter names.
         * @return Vector of parameter names
         */
        std::vector<std::string> GetParameterNames() const;

        /**
         * @brief Clears all parameters.
         */
        void Clear();

        /**
         * @brief Gets parameter count.
         * @return Number of parameters
         */
        size_t Count() const;

    private:
        std::unordered_map<std::string, ParameterValue> parameters; ///< Storage for parameter name-value pairs
    };

    // ParameterStorage template method instantiations
    extern template void ParameterStorage::Set<int>(const std::string &, int &&);
    extern template void ParameterStorage::Set<double>(const std::string &, double &&);
    extern template void ParameterStorage::Set<bool>(const std::string &, bool &&);
    extern template void ParameterStorage::Set<std::string>(const std::string &, std::string &&);
    extern template void ParameterStorage::Set<std::filesystem::path>(const std::string &, std::filesystem::path &&);
    extern template void ParameterStorage::Set<int &>(const std::string &, int &);
    extern template void ParameterStorage::Set<double &>(const std::string &, double &);
    extern template void ParameterStorage::Set<bool &>(const std::string &, bool &);
    extern template void ParameterStorage::Set<std::string &>(const std::string &, std::string &);
    extern template void ParameterStorage::Set<std::filesystem::path &>(const std::string &, std::filesystem::path &);

    extern template std::optional<int> ParameterStorage::Get<int>(const std::string &) const;
    extern template std::optional<double> ParameterStorage::Get<double>(const std::string &) const;
    extern template std::optional<bool> ParameterStorage::Get<bool>(const std::string &) const;
    extern template std::optional<std::string> ParameterStorage::Get<std::string>(const std::string &) const;
    extern template std::optional<std::filesystem::path> ParameterStorage::Get<std::filesystem::path>(
        const std::string &) const;

    extern template int ParameterStorage::GetOr<int>(const std::string &, int) const;
    extern template double ParameterStorage::GetOr<double>(const std::string &, double) const;
    extern template bool ParameterStorage::GetOr<bool>(const std::string &, bool) const;
    extern template std::string ParameterStorage::GetOr<std::string>(const std::string &, std::string) const;
    extern template std::filesystem::path ParameterStorage::GetOr<std::filesystem::path>(const std::string &,
        std::filesystem::path) const;

    extern template int ParameterStorage::GetValidated<int>(const std::string &,
        int,
        const ValidationRange<int> &,
        const std::string &) const;
    extern template double ParameterStorage::GetValidated<double>(const std::string &,
        double,
        const ValidationRange<double> &,
        const std::string &) const;

} // namespace VisionCraft::Engine