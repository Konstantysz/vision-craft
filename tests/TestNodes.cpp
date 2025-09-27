#include "VisionCraftEngine/Node.h"
#include "VisionCraftEngine/NodeParameter.h"

#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>

using namespace VisionCraft::Engine;

// Dummy derived Node for testing abstract Process()
class DummyNode : public Node
{
public:
    DummyNode(NodeId id, std::string name) : Node(id, std::move(name))
    {
    }

    void Process() override
    {
        // no-op for testing
    }
};

class NodeTest : public ::testing::Test
{
protected:
    DummyNode node{ 1, "TestNode" };
};

// ============================================================================
// Basic Node Functionality Tests
// ============================================================================

TEST_F(NodeTest, ConstructionAndAccessors)
{
    DummyNode testNode(42, "TestNode");

    EXPECT_EQ(testNode.GetId(), 42);
    EXPECT_EQ(testNode.GetName(), "TestNode");
    EXPECT_TRUE(testNode.GetParameterNames().empty());
    EXPECT_EQ(testNode.GetParameters().Count(), 0);
}

TEST_F(NodeTest, NodeConstructionEdgeCases)
{
    // Test with zero ID
    DummyNode zeroNode(0, "ZeroNode");
    EXPECT_EQ(zeroNode.GetId(), 0);
    EXPECT_EQ(zeroNode.GetName(), "ZeroNode");

    // Test with negative ID
    DummyNode negativeNode(-1, "NegativeNode");
    EXPECT_EQ(negativeNode.GetId(), -1);
    EXPECT_EQ(negativeNode.GetName(), "NegativeNode");

    // Test with empty name
    DummyNode emptyNameNode(100, "");
    EXPECT_EQ(emptyNameNode.GetId(), 100);
    EXPECT_EQ(emptyNameNode.GetName(), "");

    // Test with name containing special characters
    DummyNode specialNameNode(200, "Node-With_Special.Chars!");
    EXPECT_EQ(specialNameNode.GetId(), 200);
    EXPECT_EQ(specialNameNode.GetName(), "Node-With_Special.Chars!");
}

// ============================================================================
// Parameter Storage Tests
// ============================================================================

TEST_F(NodeTest, SetAndGetBasicParameters)
{
    // Test int parameter
    node.SetParam("int_param", 42);
    auto intVal = node.GetParam<int>("int_param");
    ASSERT_TRUE(intVal.has_value());
    EXPECT_EQ(intVal.value(), 42);

    // Test double parameter
    node.SetParam("double_param", 3.14);
    auto doubleVal = node.GetParam<double>("double_param");
    ASSERT_TRUE(doubleVal.has_value());
    EXPECT_DOUBLE_EQ(doubleVal.value(), 3.14);

    // Test bool parameter
    node.SetParam("bool_param", true);
    auto boolVal = node.GetParam<bool>("bool_param");
    ASSERT_TRUE(boolVal.has_value());
    EXPECT_TRUE(boolVal.value());

    // Test string parameter
    node.SetParam("string_param", std::string("test_value"));
    auto stringVal = node.GetParam<std::string>("string_param");
    ASSERT_TRUE(stringVal.has_value());
    EXPECT_EQ(stringVal.value(), "test_value");

    // Test filesystem::path parameter
    std::filesystem::path testPath("/test/path.txt");
    node.SetParam("path_param", testPath);
    auto pathVal = node.GetParam<std::filesystem::path>("path_param");
    ASSERT_TRUE(pathVal.has_value());
    EXPECT_EQ(pathVal.value(), testPath);
}

TEST_F(NodeTest, GetParamOrWithDefaults)
{
    // Test with existing parameter
    node.SetParam("existing", 100);
    EXPECT_EQ(node.GetParamOr("existing", 200), 100);

    // Test with non-existing parameter
    EXPECT_EQ(node.GetParamOr("non_existing", 300), 300);

    // Test with wrong type (should return default)
    node.SetParam("string_param", std::string("hello"));
    EXPECT_EQ(node.GetParamOr<int>("string_param", 400), 400);
}

TEST_F(NodeTest, NonExistentParameters)
{
    // Test getting non-existent parameter
    EXPECT_FALSE(node.GetParam<std::string>("nonexistent").has_value());
    EXPECT_FALSE(node.GetParam<int>("").has_value());

    // Add a parameter
    node.SetParam("real", std::string("value"));

    // Non-existent should still return nullopt
    EXPECT_FALSE(node.GetParam<std::string>("fake").has_value());
    EXPECT_TRUE(node.GetParam<std::string>("real").has_value());
}

TEST_F(NodeTest, ParameterOverwriting)
{
    // Set initial value
    node.SetParam("param", std::string("initial"));
    EXPECT_EQ(node.GetParam<std::string>("param").value(), "initial");
    EXPECT_EQ(node.GetParameterNames().size(), 1);

    // Overwrite with new value
    node.SetParam("param", std::string("updated"));
    EXPECT_EQ(node.GetParam<std::string>("param").value(), "updated");

    // Should still have only one parameter (not duplicated)
    EXPECT_EQ(node.GetParameterNames().size(), 1);
    EXPECT_TRUE(node.HasParameter("param"));
}

TEST_F(NodeTest, MultipleParameters)
{
    // Set multiple parameters with different types
    node.SetParam("threshold", 0.5);
    node.SetParam("iterations", 100);
    node.SetParam("enableFlag", true);
    node.SetParam("algorithm", std::string("gaussian"));

    // Verify all parameters exist and have correct values
    EXPECT_EQ(node.GetParam<double>("threshold").value(), 0.5);
    EXPECT_EQ(node.GetParam<int>("iterations").value(), 100);
    EXPECT_EQ(node.GetParam<bool>("enableFlag").value(), true);
    EXPECT_EQ(node.GetParam<std::string>("algorithm").value(), "gaussian");

    // Verify GetParameterNames() returns all parameters
    const auto paramNames = node.GetParameterNames();
    EXPECT_EQ(paramNames.size(), 4);

    // Verify parameter names exist (names are sorted)
    EXPECT_TRUE(std::find(paramNames.begin(), paramNames.end(), "threshold") != paramNames.end());
    EXPECT_TRUE(std::find(paramNames.begin(), paramNames.end(), "iterations") != paramNames.end());
    EXPECT_TRUE(std::find(paramNames.begin(), paramNames.end(), "enableFlag") != paramNames.end());
    EXPECT_TRUE(std::find(paramNames.begin(), paramNames.end(), "algorithm") != paramNames.end());
}

TEST_F(NodeTest, ParameterCaseSensitivity)
{
    // Set parameters with different cases
    node.SetParam("Threshold", std::string("0.5"));
    node.SetParam("threshold", std::string("0.7"));
    node.SetParam("THRESHOLD", std::string("0.9"));

    // Should be treated as separate parameters
    EXPECT_EQ(node.GetParam<std::string>("Threshold").value(), "0.5");
    EXPECT_EQ(node.GetParam<std::string>("threshold").value(), "0.7");
    EXPECT_EQ(node.GetParam<std::string>("THRESHOLD").value(), "0.9");
    EXPECT_EQ(node.GetParameterNames().size(), 3);
}

TEST_F(NodeTest, ParameterEdgeCases)
{
    // Test empty parameter value
    node.SetParam("empty", std::string(""));
    EXPECT_EQ(node.GetParam<std::string>("empty").value(), "");

    // Test parameter with spaces and special characters
    node.SetParam("special", std::string("Hello World! @#$%^&*()"));
    EXPECT_EQ(node.GetParam<std::string>("special").value(), "Hello World! @#$%^&*()");

    // Test very long parameter name and value
    std::string longName = std::string(100, 'a');
    std::string longValue = std::string(1000, 'b');
    node.SetParam(longName, longValue);
    EXPECT_EQ(node.GetParam<std::string>(longName).value(), longValue);

    // Test parameter name with special characters
    node.SetParam("param_with-special.chars", std::string("value"));
    EXPECT_EQ(node.GetParam<std::string>("param_with-special.chars").value(), "value");
}

// ============================================================================
// Advanced Parameter Methods Tests
// ============================================================================

TEST_F(NodeTest, BooleanParameterParsing)
{
    // Test direct boolean
    node.SetParam("bool_direct", true);
    EXPECT_TRUE(node.GetBoolParam("bool_direct", false));

    // Test string parsing - true values
    node.SetParam("true_string", std::string("true"));
    EXPECT_TRUE(node.GetBoolParam("true_string", false));

    node.SetParam("one_string", std::string("1"));
    EXPECT_TRUE(node.GetBoolParam("one_string", false));

    node.SetParam("yes_string", std::string("yes"));
    EXPECT_TRUE(node.GetBoolParam("yes_string", false));

    node.SetParam("on_string", std::string("on"));
    EXPECT_TRUE(node.GetBoolParam("on_string", false));

    // Test string parsing - false values
    node.SetParam("false_string", std::string("false"));
    EXPECT_FALSE(node.GetBoolParam("false_string", true));

    node.SetParam("zero_string", std::string("0"));
    EXPECT_FALSE(node.GetBoolParam("zero_string", true));

    // Test case insensitivity
    node.SetParam("upper_true", std::string("TRUE"));
    EXPECT_TRUE(node.GetBoolParam("upper_true", false));

    // Test invalid string (should return default)
    node.SetParam("invalid_bool", std::string("maybe"));
    EXPECT_TRUE(node.GetBoolParam("invalid_bool", true));
    EXPECT_FALSE(node.GetBoolParam("invalid_bool", false));

    // Test non-existent parameter
    EXPECT_TRUE(node.GetBoolParam("non_existent", true));
    EXPECT_FALSE(node.GetBoolParam("non_existent", false));
}

TEST_F(NodeTest, ValidatedNumericParameters)
{
    ValidationRange<int> intRange(0, 100);
    ValidationRange<double> doubleRange(0.0, 1.0);

    // Test valid values
    node.SetParam("int_param", 50);
    EXPECT_EQ(node.GetValidatedParam("int_param", 10, intRange), 50);

    node.SetParam("double_param", 0.75);
    EXPECT_DOUBLE_EQ(node.GetValidatedParam("double_param", 0.5, doubleRange), 0.75);

    // Test clamping (values outside range)
    node.SetParam("high_int", 150);
    EXPECT_EQ(node.GetValidatedParam("high_int", 10, intRange), 100);

    node.SetParam("low_double", -0.5);
    EXPECT_DOUBLE_EQ(node.GetValidatedParam("low_double", 0.5, doubleRange), 0.0);

    // Test default value for non-existent parameter
    EXPECT_EQ(node.GetValidatedParam<int>("non_existent", 25, intRange), 25);
}

TEST_F(NodeTest, ValidatedStringParameters)
{
    StringValidation validation({ "option1", "option2", "option3" });

    // Test valid value
    node.SetParam("string_param", std::string("option1"));
    EXPECT_EQ(node.GetValidatedString("string_param", "default", validation), "option1");

    // Test invalid value
    node.SetParam("invalid_string", std::string("invalid"));
    EXPECT_EQ(node.GetValidatedString("invalid_string", "default", validation), "default");

    // Test non-existent parameter
    EXPECT_EQ(node.GetValidatedString("non_existent", "default", validation), "default");

    // Test no validation (empty allowed values)
    StringValidation noValidation;
    EXPECT_EQ(node.GetValidatedString("invalid_string", "default", noValidation), "invalid");
}

TEST_F(NodeTest, PathParameters)
{
    // Test direct path
    std::filesystem::path directPath("/direct/path.txt");
    node.SetParam("direct_path", directPath);
    EXPECT_EQ(node.GetPath("direct_path"), directPath);

    // Test string conversion
    node.SetParam("string_path", std::string("/string/path.txt"));
    EXPECT_EQ(node.GetPath("string_path"), std::filesystem::path("/string/path.txt"));

    // Test default fallback
    std::filesystem::path defaultPath("/default/path.txt");
    EXPECT_EQ(node.GetPath("non_existent", defaultPath), defaultPath);

    // Test wrong type fallback
    node.SetParam("int_param", 42);
    EXPECT_EQ(node.GetPath("int_param", defaultPath), defaultPath);
}

TEST_F(NodeTest, FilePathValidation)
{
    FilePathValidation validation;
    validation.mustExist = false; // Don't check file existence for tests
    validation.allowedExtensions = { ".txt", ".cpp" };

    // Test valid extension
    node.SetParam("valid_path", std::filesystem::path("/path/file.txt"));
    EXPECT_TRUE(node.ValidateFilePath("valid_path", validation));

    // Test invalid extension
    node.SetParam("invalid_ext", std::filesystem::path("/path/file.xyz"));
    EXPECT_FALSE(node.ValidateFilePath("invalid_ext", validation));

    // Test no extension validation
    FilePathValidation noExtValidation;
    noExtValidation.mustExist = false;
    // No allowed extensions specified
    EXPECT_TRUE(node.ValidateFilePath("invalid_ext", noExtValidation));

    // Test non-existent parameter
    EXPECT_FALSE(node.ValidateFilePath("non_existent", validation));
}

// ============================================================================
// Parameter Storage Integration Tests
// ============================================================================

TEST_F(NodeTest, ParameterStorageAccess)
{
    // Test direct access to parameter storage
    auto &params = node.GetParameters();
    params.Set("direct_param", 123);

    // Verify through Node interface
    EXPECT_EQ(node.GetParam<int>("direct_param").value(), 123);
    EXPECT_TRUE(node.HasParameter("direct_param"));

    // Test const access
    const auto &constNode = node;
    const auto &constParams = constNode.GetParameters();
    auto value = constParams.Get<int>("direct_param");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 123);
}

TEST_F(NodeTest, ParameterUtilityMethods)
{
    // Test HasParameter
    EXPECT_FALSE(node.HasParameter("test_param"));
    node.SetParam("test_param", 123);
    EXPECT_TRUE(node.HasParameter("test_param"));

    // Test parameter count through GetParameterNames
    EXPECT_EQ(node.GetParameterNames().size(), 1);
    node.SetParam("another_param", std::string("value"));
    EXPECT_EQ(node.GetParameterNames().size(), 2);

    // Test parameter names are sorted
    auto names = node.GetParameterNames();
    EXPECT_TRUE(std::is_sorted(names.begin(), names.end()));
}