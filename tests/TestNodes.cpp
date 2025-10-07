#include "VisionCraftEngine/Node.h"
#include "VisionCraftEngine/NodeData.h"

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

    std::string GetType() const
    {
        return "DummyNode";
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
// Slot Functionality Tests
// ============================================================================

TEST_F(NodeTest, CreateInputSlotWithoutDefault)
{
    node.CreateInputSlot("Input");

    EXPECT_TRUE(node.HasInputSlot("Input"));
    EXPECT_FALSE(node.HasInputSlot("NonExistent"));

    const auto &slot = node.GetInputSlot("Input");
    EXPECT_FALSE(slot.HasData());
    EXPECT_FALSE(slot.HasDefaultValue());
}

TEST_F(NodeTest, CreateInputSlotWithDefault)
{
    node.CreateInputSlot("Threshold", 127.0);

    EXPECT_TRUE(node.HasInputSlot("Threshold"));

    const auto &slot = node.GetInputSlot("Threshold");
    EXPECT_FALSE(slot.HasData());
    EXPECT_TRUE(slot.HasDefaultValue());

    auto defaultVal = slot.GetDefaultValue<double>();
    ASSERT_TRUE(defaultVal.has_value());
    EXPECT_DOUBLE_EQ(defaultVal.value(), 127.0);
}

TEST_F(NodeTest, CreateInputSlotWithDifferentTypes)
{
    node.CreateInputSlot("IntParam", 42);
    node.CreateInputSlot("BoolParam", true);
    node.CreateInputSlot("StringParam", std::string("test"));
    node.CreateInputSlot("PathParam", std::filesystem::path("/test"));

    EXPECT_TRUE(node.HasInputSlot("IntParam"));
    EXPECT_TRUE(node.HasInputSlot("BoolParam"));
    EXPECT_TRUE(node.HasInputSlot("StringParam"));
    EXPECT_TRUE(node.HasInputSlot("PathParam"));

    EXPECT_EQ(node.GetInputSlot("IntParam").GetDefaultValue<int>().value(), 42);
    EXPECT_TRUE(node.GetInputSlot("BoolParam").GetDefaultValue<bool>().value());
    EXPECT_EQ(node.GetInputSlot("StringParam").GetDefaultValue<std::string>().value(), "test");
    EXPECT_EQ(node.GetInputSlot("PathParam").GetDefaultValue<std::filesystem::path>().value(),
        std::filesystem::path("/test"));
}

TEST_F(NodeTest, CreateOutputSlot)
{
    node.CreateOutputSlot("Output");

    EXPECT_TRUE(node.HasOutputSlot("Output"));
    EXPECT_FALSE(node.HasOutputSlot("NonExistent"));

    const auto &slot = node.GetOutputSlot("Output");
    EXPECT_FALSE(slot.HasData());
}

TEST_F(NodeTest, SetAndGetInputSlotData)
{
    node.CreateInputSlot("Input");
    node.SetInputSlotData("Input", NodeData(42));

    const auto &slot = node.GetInputSlot("Input");
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<int>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 42);
}

TEST_F(NodeTest, SetAndGetOutputSlotData)
{
    node.CreateOutputSlot("Output");
    node.SetOutputSlotData("Output", NodeData(3.14));

    const auto &slot = node.GetOutputSlot("Output");
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<double>();
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.value(), 3.14);
}

TEST_F(NodeTest, ClearInputSlot)
{
    node.CreateInputSlot("Input");
    node.SetInputSlotData("Input", NodeData(100));

    EXPECT_TRUE(node.GetInputSlot("Input").HasData());

    node.ClearInputSlot("Input");
    EXPECT_FALSE(node.GetInputSlot("Input").HasData());
}

TEST_F(NodeTest, ClearOutputSlot)
{
    node.CreateOutputSlot("Output");
    node.SetOutputSlotData("Output", NodeData(200));

    EXPECT_TRUE(node.GetOutputSlot("Output").HasData());

    node.ClearOutputSlot("Output");
    EXPECT_FALSE(node.GetOutputSlot("Output").HasData());
}

TEST_F(NodeTest, GetInputValueWithDefault)
{
    node.CreateInputSlot("Threshold", 127.0);

    // No connected data, should return default
    auto value = node.GetInputValue<double>("Threshold");
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.value(), 127.0);
}

TEST_F(NodeTest, GetInputValuePrefersConnected)
{
    node.CreateInputSlot("Threshold", 127.0);
    node.SetInputSlotData("Threshold", NodeData(200.0));

    // Connected data should override default
    auto value = node.GetInputValue<double>("Threshold");
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.value(), 200.0);
}

TEST_F(NodeTest, SetInputSlotDefault)
{
    node.CreateInputSlot("Param", 100);

    // Change default value
    node.SetInputSlotDefault("Param", NodeData(200));

    auto value = node.GetInputValue<int>("Param");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 200);
}

TEST_F(NodeTest, IsInputSlotConnected)
{
    node.CreateInputSlot("Input", 42);

    EXPECT_FALSE(node.IsInputSlotConnected("Input"));

    node.SetInputSlotData("Input", NodeData(100));
    EXPECT_TRUE(node.IsInputSlotConnected("Input"));

    node.ClearInputSlot("Input");
    EXPECT_FALSE(node.IsInputSlotConnected("Input"));
}

TEST_F(NodeTest, MultipleSlots)
{
    node.CreateInputSlot("Input1");
    node.CreateInputSlot("Input2", 50);
    node.CreateInputSlot("Input3", std::string("default"));
    node.CreateOutputSlot("Output1");
    node.CreateOutputSlot("Output2");

    EXPECT_TRUE(node.HasInputSlot("Input1"));
    EXPECT_TRUE(node.HasInputSlot("Input2"));
    EXPECT_TRUE(node.HasInputSlot("Input3"));
    EXPECT_TRUE(node.HasOutputSlot("Output1"));
    EXPECT_TRUE(node.HasOutputSlot("Output2"));
}

TEST_F(NodeTest, SlotDoesNotExistThrows)
{
    EXPECT_THROW(node.GetInputSlot("NonExistent"), std::out_of_range);
    EXPECT_THROW(node.GetOutputSlot("NonExistent"), std::out_of_range);
    EXPECT_THROW(node.SetInputSlotData("NonExistent", NodeData(42)), std::out_of_range);
    EXPECT_THROW(node.SetOutputSlotData("NonExistent", NodeData(42)), std::out_of_range);
}