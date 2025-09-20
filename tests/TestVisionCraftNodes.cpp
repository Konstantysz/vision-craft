#include "Node.h"

#include <gtest/gtest.h>

// Dummy derived Node for testing abstract Process()
class DummyNode : public VisionCraft::Engine::Node
{
public:
    DummyNode(VisionCraft::Engine::NodeId id, std::string name) : Node(id, std::move(name))
    {
    }

    void Process() override
    {
    } // no-op for testing
};

TEST(NodeTest, ConstructionAndAccessors)
{
    DummyNode node(42, "TestNode");

    EXPECT_EQ(node.GetId(), 42);
    EXPECT_EQ(node.GetName(), "TestNode");
    EXPECT_TRUE(node.GetParams().empty());
}

TEST(NodeTest, SetAndGetParamValue)
{
    DummyNode node(1, "ParamNode");

    // initially no params
    EXPECT_FALSE(node.GetParamValue("threshold").has_value());

    node.SetParamValue("threshold", "0.5");

    auto val = node.GetParamValue("threshold");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), "0.5");

    // modify param
    node.SetParamValue("threshold", "0.7");
    EXPECT_EQ(node.GetParamValue("threshold").value(), "0.7");
}

TEST(NodeTest, MultipleParameters)
{
    DummyNode node(2, "MultiParamNode");

    // Set multiple parameters
    node.SetParamValue("threshold", "0.5");
    node.SetParamValue("iterations", "100");
    node.SetParamValue("enableFlag", "true");

    // Verify all parameters exist and have correct values
    EXPECT_EQ(node.GetParamValue("threshold").value(), "0.5");
    EXPECT_EQ(node.GetParamValue("iterations").value(), "100");
    EXPECT_EQ(node.GetParamValue("enableFlag").value(), "true");

    // Verify GetParams() returns all parameters
    const auto& params = node.GetParams();
    EXPECT_EQ(params.size(), 3);

    // Verify parameter names and values in collection
    bool foundThreshold = false, foundIterations = false, foundFlag = false;
    for (const auto& param : params)
    {
        if (param.name == "threshold" && param.value == "0.5") foundThreshold = true;
        if (param.name == "iterations" && param.value == "100") foundIterations = true;
        if (param.name == "enableFlag" && param.value == "true") foundFlag = true;
    }
    EXPECT_TRUE(foundThreshold);
    EXPECT_TRUE(foundIterations);
    EXPECT_TRUE(foundFlag);
}

TEST(NodeTest, ParameterEdgeCases)
{
    DummyNode node(3, "EdgeCaseNode");

    // Test empty parameter value
    node.SetParamValue("empty", "");
    EXPECT_EQ(node.GetParamValue("empty").value(), "");

    // Test parameter with spaces and special characters
    node.SetParamValue("special", "Hello World! @#$%^&*()");
    EXPECT_EQ(node.GetParamValue("special").value(), "Hello World! @#$%^&*()");

    // Test very long parameter name and value
    std::string longName = std::string(100, 'a');
    std::string longValue = std::string(1000, 'b');
    node.SetParamValue(longName, longValue);
    EXPECT_EQ(node.GetParamValue(longName).value(), longValue);

    // Test parameter name with special characters
    node.SetParamValue("param_with-special.chars", "value");
    EXPECT_EQ(node.GetParamValue("param_with-special.chars").value(), "value");
}

TEST(NodeTest, ParameterOverwriting)
{
    DummyNode node(4, "OverwriteNode");

    // Set initial value
    node.SetParamValue("param", "initial");
    EXPECT_EQ(node.GetParamValue("param").value(), "initial");
    EXPECT_EQ(node.GetParams().size(), 1);

    // Overwrite with new value
    node.SetParamValue("param", "updated");
    EXPECT_EQ(node.GetParamValue("param").value(), "updated");

    // Should still have only one parameter (not duplicated)
    EXPECT_EQ(node.GetParams().size(), 1);
    EXPECT_EQ(node.GetParams()[0].name, "param");
    EXPECT_EQ(node.GetParams()[0].value, "updated");
}

TEST(NodeTest, NonExistentParameter)
{
    DummyNode node(5, "NonExistentNode");

    // Test getting non-existent parameter
    EXPECT_FALSE(node.GetParamValue("nonexistent").has_value());
    EXPECT_FALSE(node.GetParamValue("").has_value());

    // Add some parameters
    node.SetParamValue("real", "value");

    // Non-existent should still return nullopt
    EXPECT_FALSE(node.GetParamValue("fake").has_value());
    EXPECT_TRUE(node.GetParamValue("real").has_value());
}

TEST(NodeTest, NodeConstructionEdgeCases)
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

TEST(NodeTest, ParameterCaseSensitivity)
{
    DummyNode node(6, "CaseNode");

    // Set parameters with different cases
    node.SetParamValue("Threshold", "0.5");
    node.SetParamValue("threshold", "0.7");
    node.SetParamValue("THRESHOLD", "0.9");

    // Should be treated as separate parameters
    EXPECT_EQ(node.GetParamValue("Threshold").value(), "0.5");
    EXPECT_EQ(node.GetParamValue("threshold").value(), "0.7");
    EXPECT_EQ(node.GetParamValue("THRESHOLD").value(), "0.9");
    EXPECT_EQ(node.GetParams().size(), 3);
}
