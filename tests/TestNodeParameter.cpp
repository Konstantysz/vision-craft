#include "VisionCraftEngine/NodeParameter.h"

#include <filesystem>
#include <gtest/gtest.h>

using namespace VisionCraft::Engine;

class NodeParameterTest : public ::testing::Test
{
protected:
    ParameterStorage storage;
};

TEST_F(NodeParameterTest, SetAndGetBasicTypes)
{
    // Test int
    storage.Set("int_param", 42);
    auto intVal = storage.Get<int>("int_param");
    ASSERT_TRUE(intVal.has_value());
    EXPECT_EQ(intVal.value(), 42);

    // Test double
    storage.Set("double_param", 3.14);
    auto doubleVal = storage.Get<double>("double_param");
    ASSERT_TRUE(doubleVal.has_value());
    EXPECT_DOUBLE_EQ(doubleVal.value(), 3.14);

    // Test bool
    storage.Set("bool_param", true);
    auto boolVal = storage.Get<bool>("bool_param");
    ASSERT_TRUE(boolVal.has_value());
    EXPECT_TRUE(boolVal.value());

    // Test string
    storage.Set("string_param", std::string("test_string"));
    auto stringVal = storage.Get<std::string>("string_param");
    ASSERT_TRUE(stringVal.has_value());
    EXPECT_EQ(stringVal.value(), "test_string");

    // Test filesystem::path
    std::filesystem::path testPath("/path/to/file.txt");
    storage.Set("path_param", testPath);
    auto pathVal = storage.Get<std::filesystem::path>("path_param");
    ASSERT_TRUE(pathVal.has_value());
    EXPECT_EQ(pathVal.value(), testPath);
}

TEST_F(NodeParameterTest, GetNonExistentParameter)
{
    auto result = storage.Get<int>("non_existent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NodeParameterTest, GetWrongType)
{
    storage.Set("int_param", 42);
    auto result = storage.Get<std::string>("int_param");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NodeParameterTest, GetOrWithDefault)
{
    // Test with existing parameter
    storage.Set("existing", 100);
    EXPECT_EQ(storage.GetOr("existing", 200), 100);

    // Test with non-existing parameter
    EXPECT_EQ(storage.GetOr("non_existing", 300), 300);

    // Test with wrong type (should return default)
    storage.Set("string_param", std::string("hello"));
    EXPECT_EQ(storage.GetOr<int>("string_param", 400), 400);
}

TEST_F(NodeParameterTest, BooleanParsing)
{
    // Test direct boolean
    storage.Set("bool_direct", true);
    EXPECT_TRUE(storage.GetBool("bool_direct", false));

    // Test string parsing - true values
    storage.Set("true_string", std::string("true"));
    EXPECT_TRUE(storage.GetBool("true_string", false));

    storage.Set("one_string", std::string("1"));
    EXPECT_TRUE(storage.GetBool("one_string", false));

    storage.Set("yes_string", std::string("yes"));
    EXPECT_TRUE(storage.GetBool("yes_string", false));

    storage.Set("on_string", std::string("on"));
    EXPECT_TRUE(storage.GetBool("on_string", false));

    // Test string parsing - false values
    storage.Set("false_string", std::string("false"));
    EXPECT_FALSE(storage.GetBool("false_string", true));

    storage.Set("zero_string", std::string("0"));
    EXPECT_FALSE(storage.GetBool("zero_string", true));

    storage.Set("no_string", std::string("no"));
    EXPECT_FALSE(storage.GetBool("no_string", true));

    storage.Set("off_string", std::string("off"));
    EXPECT_FALSE(storage.GetBool("off_string", true));

    // Test case insensitivity
    storage.Set("upper_true", std::string("TRUE"));
    EXPECT_TRUE(storage.GetBool("upper_true", false));

    storage.Set("mixed_false", std::string("FaLsE"));
    EXPECT_FALSE(storage.GetBool("mixed_false", true));

    // Test invalid string (should return default)
    storage.Set("invalid_bool", std::string("maybe"));
    EXPECT_TRUE(storage.GetBool("invalid_bool", true));
    EXPECT_FALSE(storage.GetBool("invalid_bool", false));

    // Test non-existent parameter
    EXPECT_TRUE(storage.GetBool("non_existent", true));
    EXPECT_FALSE(storage.GetBool("non_existent", false));
}

TEST_F(NodeParameterTest, PathHandling)
{
    // Test direct path
    std::filesystem::path directPath("/direct/path.txt");
    storage.Set("direct_path", directPath);
    EXPECT_EQ(storage.GetPath("direct_path"), directPath);

    // Test string conversion
    storage.Set("string_path", std::string("/string/path.txt"));
    EXPECT_EQ(storage.GetPath("string_path"), std::filesystem::path("/string/path.txt"));

    // Test default fallback
    std::filesystem::path defaultPath("/default/path.txt");
    EXPECT_EQ(storage.GetPath("non_existent", defaultPath), defaultPath);

    // Test wrong type fallback
    storage.Set("int_param", 42);
    EXPECT_EQ(storage.GetPath("int_param", defaultPath), defaultPath);
}

TEST_F(NodeParameterTest, UtilityMethods)
{
    // Test HasParameter
    EXPECT_FALSE(storage.HasParameter("test_param"));
    storage.Set("test_param", 123);
    EXPECT_TRUE(storage.HasParameter("test_param"));

    // Test Count
    EXPECT_EQ(storage.Count(), 1);
    storage.Set("another_param", std::string("value"));
    EXPECT_EQ(storage.Count(), 2);

    // Test GetParameterNames
    auto names = storage.GetParameterNames();
    EXPECT_EQ(names.size(), 2);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "test_param") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "another_param") != names.end());

    // Test Clear
    storage.Clear();
    EXPECT_EQ(storage.Count(), 0);
    EXPECT_FALSE(storage.HasParameter("test_param"));
    EXPECT_FALSE(storage.HasParameter("another_param"));
}

TEST(ValidationRangeTest, IntValidation)
{
    ValidationRange<int> range(10, 20);

    // Test value within range
    EXPECT_EQ(range.ValidateAndClamp(15, "test_param", "TestNode"), 15);

    // Test value below minimum (should clamp)
    EXPECT_EQ(range.ValidateAndClamp(5, "test_param", "TestNode"), 10);

    // Test value above maximum (should clamp)
    EXPECT_EQ(range.ValidateAndClamp(25, "test_param", "TestNode"), 20);

    // Test boundary values
    EXPECT_EQ(range.ValidateAndClamp(10, "test_param", "TestNode"), 10);
    EXPECT_EQ(range.ValidateAndClamp(20, "test_param", "TestNode"), 20);
}

TEST(ValidationRangeTest, DoubleValidation)
{
    ValidationRange<double> range(0.0, 1.0);

    // Test value within range
    EXPECT_DOUBLE_EQ(range.ValidateAndClamp(0.5, "test_param", "TestNode"), 0.5);

    // Test value below minimum
    EXPECT_DOUBLE_EQ(range.ValidateAndClamp(-0.1, "test_param", "TestNode"), 0.0);

    // Test value above maximum
    EXPECT_DOUBLE_EQ(range.ValidateAndClamp(1.5, "test_param", "TestNode"), 1.0);
}

TEST(ValidationRangeTest, MinOnlyValidation)
{
    ValidationRange<int> range(10); // Only minimum

    // Test value above minimum
    EXPECT_EQ(range.ValidateAndClamp(15, "test_param", "TestNode"), 15);
    EXPECT_EQ(range.ValidateAndClamp(100, "test_param", "TestNode"), 100);

    // Test value below minimum
    EXPECT_EQ(range.ValidateAndClamp(5, "test_param", "TestNode"), 10);
}

TEST(ValidationRangeTest, NoLimitsValidation)
{
    ValidationRange<int> range; // No limits

    // All values should pass through unchanged
    EXPECT_EQ(range.ValidateAndClamp(-1000, "test_param", "TestNode"), -1000);
    EXPECT_EQ(range.ValidateAndClamp(0, "test_param", "TestNode"), 0);
    EXPECT_EQ(range.ValidateAndClamp(1000, "test_param", "TestNode"), 1000);
}

TEST_F(NodeParameterTest, GetValidatedNumeric)
{
    ValidationRange<int> intRange(0, 100);
    ValidationRange<double> doubleRange(0.0, 1.0);

    // Test valid values
    storage.Set("int_param", 50);
    EXPECT_EQ(storage.GetValidated("int_param", 10, intRange, "TestNode"), 50);

    storage.Set("double_param", 0.75);
    EXPECT_DOUBLE_EQ(storage.GetValidated("double_param", 0.5, doubleRange, "TestNode"), 0.75);

    // Test clamping
    storage.Set("high_int", 150);
    EXPECT_EQ(storage.GetValidated("high_int", 10, intRange, "TestNode"), 100);

    storage.Set("low_double", -0.5);
    EXPECT_DOUBLE_EQ(storage.GetValidated("low_double", 0.5, doubleRange, "TestNode"), 0.0);

    // Test default value
    EXPECT_EQ(storage.GetValidated<int>("non_existent", 25, intRange, "TestNode"), 25);
}

TEST(StringValidationTest, AllowedValues)
{
    StringValidation validation({ "red", "green", "blue" }, true);

    // Test valid values
    EXPECT_EQ(validation.ValidateOrDefault("red", "default", "color", "TestNode"), "red");
    EXPECT_EQ(validation.ValidateOrDefault("green", "default", "color", "TestNode"), "green");
    EXPECT_EQ(validation.ValidateOrDefault("blue", "default", "color", "TestNode"), "blue");

    // Test invalid value
    EXPECT_EQ(validation.ValidateOrDefault("yellow", "default", "color", "TestNode"), "default");
}

TEST(StringValidationTest, CaseInsensitive)
{
    StringValidation validation({ "Red", "Green", "Blue" }, false);

    // Test case variations
    EXPECT_EQ(validation.ValidateOrDefault("red", "default", "color", "TestNode"), "red");
    EXPECT_EQ(validation.ValidateOrDefault("RED", "default", "color", "TestNode"), "RED");
    EXPECT_EQ(validation.ValidateOrDefault("Green", "default", "color", "TestNode"), "Green");
    EXPECT_EQ(validation.ValidateOrDefault("bLuE", "default", "color", "TestNode"), "bLuE");

    // Test invalid value
    EXPECT_EQ(validation.ValidateOrDefault("yellow", "default", "color", "TestNode"), "default");
}

TEST(StringValidationTest, CaseSensitive)
{
    StringValidation validation({ "Red", "Green", "Blue" }, true);

    // Test exact matches
    EXPECT_EQ(validation.ValidateOrDefault("Red", "default", "color", "TestNode"), "Red");
    EXPECT_EQ(validation.ValidateOrDefault("Green", "default", "color", "TestNode"), "Green");

    // Test case mismatches
    EXPECT_EQ(validation.ValidateOrDefault("red", "default", "color", "TestNode"), "default");
    EXPECT_EQ(validation.ValidateOrDefault("GREEN", "default", "color", "TestNode"), "default");
}

TEST(StringValidationTest, NoValidation)
{
    StringValidation validation; // Empty allowed values

    // Any value should be accepted
    EXPECT_EQ(validation.ValidateOrDefault("anything", "default", "param", "TestNode"), "anything");
    EXPECT_EQ(validation.ValidateOrDefault("", "default", "param", "TestNode"), "");
}

TEST_F(NodeParameterTest, GetValidatedString)
{
    StringValidation validation({ "option1", "option2", "option3" });

    // Test valid value
    storage.Set("string_param", std::string("option1"));
    EXPECT_EQ(storage.GetValidatedString("string_param", "default", validation, "TestNode"), "option1");

    // Test invalid value
    storage.Set("invalid_string", std::string("invalid"));
    EXPECT_EQ(storage.GetValidatedString("invalid_string", "default", validation, "TestNode"), "default");

    // Test non-existent parameter
    EXPECT_EQ(storage.GetValidatedString("non_existent", "default", validation, "TestNode"), "default");
}

TEST(FilePathValidationTest, EmptyPaths)
{
    FilePathValidation allowEmpty(true, true);
    FilePathValidation disallowEmpty(true, false);

    std::filesystem::path emptyPath;

    EXPECT_TRUE(allowEmpty.ValidatePath(emptyPath, "path_param", "TestNode"));
    EXPECT_FALSE(disallowEmpty.ValidatePath(emptyPath, "path_param", "TestNode"));
}

TEST(FilePathValidationTest, Extensions)
{
    FilePathValidation validation;
    validation.allowedExtensions = { ".txt", ".md", ".cpp" };
    validation.mustExist = false; // Don't check existence for this test

    // Test valid extensions
    EXPECT_TRUE(validation.ValidatePath("/path/file.txt", "path_param", "TestNode"));
    EXPECT_TRUE(validation.ValidatePath("/path/file.md", "path_param", "TestNode"));
    EXPECT_TRUE(validation.ValidatePath("/path/file.cpp", "path_param", "TestNode"));

    // Test invalid extension
    EXPECT_FALSE(validation.ValidatePath("/path/file.xyz", "path_param", "TestNode"));

    // Test case insensitive extension matching
    EXPECT_TRUE(validation.ValidatePath("/path/file.TXT", "path_param", "TestNode"));
    EXPECT_TRUE(validation.ValidatePath("/path/file.Cpp", "path_param", "TestNode"));
}

TEST(FilePathValidationTest, NoExtensionValidation)
{
    FilePathValidation validation;
    validation.mustExist = false;
    // No allowed extensions specified

    // Any extension should be allowed
    EXPECT_TRUE(validation.ValidatePath("/path/file.txt", "path_param", "TestNode"));
    EXPECT_TRUE(validation.ValidatePath("/path/file.xyz", "path_param", "TestNode"));
    EXPECT_TRUE(validation.ValidatePath("/path/file", "path_param", "TestNode"));
}

TEST_F(NodeParameterTest, ValidateFilePath)
{
    FilePathValidation validation;
    validation.mustExist = false; // Don't check file existence
    validation.allowedExtensions = { ".txt" };

    // Test valid path
    storage.Set("valid_path", std::filesystem::path("/path/file.txt"));
    EXPECT_TRUE(storage.ValidateFilePath("valid_path", validation, "TestNode"));

    // Test invalid extension
    storage.Set("invalid_ext", std::filesystem::path("/path/file.xyz"));
    EXPECT_FALSE(storage.ValidateFilePath("invalid_ext", validation, "TestNode"));

    // Test non-existent parameter
    EXPECT_FALSE(storage.ValidateFilePath("non_existent", validation, "TestNode"));
}