#include "VisionCraftEngine/NodeData.h"
#include "VisionCraftEngine/Slot.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

using namespace VisionCraft::Engine;

class SlotTest : public ::testing::Test
{
protected:
    Slot slot;
};

// ============================================================================
// Basic Slot Functionality Tests
// ============================================================================

TEST_F(SlotTest, DefaultConstruction)
{
    Slot emptySlot;
    EXPECT_FALSE(emptySlot.HasData());
    EXPECT_FALSE(emptySlot.HasDefaultValue());
    EXPECT_FALSE(emptySlot.IsConnected());
    EXPECT_EQ(emptySlot.GetTypeIndex(), 0); // std::monostate
}

TEST_F(SlotTest, ConstructionWithDefault)
{
    Slot slotWithDefault(NodeData(42));
    EXPECT_FALSE(slotWithDefault.HasData());
    EXPECT_TRUE(slotWithDefault.HasDefaultValue());
    EXPECT_FALSE(slotWithDefault.IsConnected());

    auto defaultVal = slotWithDefault.GetDefaultValue<int>();
    ASSERT_TRUE(defaultVal.has_value());
    EXPECT_EQ(defaultVal.value(), 42);
}

// ============================================================================
// Data Storage Tests
// ============================================================================

TEST_F(SlotTest, SetAndGetInt)
{
    slot.SetData(123);
    EXPECT_TRUE(slot.HasData());
    EXPECT_TRUE(slot.IsConnected());

    auto value = slot.GetData<int>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 123);
}

TEST_F(SlotTest, SetAndGetDouble)
{
    slot.SetData(3.14159);
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<double>();
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(value.value(), 3.14159);
}

TEST_F(SlotTest, SetAndGetBool)
{
    slot.SetData(true);
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<bool>();
    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(value.value());
}

TEST_F(SlotTest, SetAndGetString)
{
    slot.SetData(std::string("test_string"));
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<std::string>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "test_string");
}

TEST_F(SlotTest, SetAndGetPath)
{
    std::filesystem::path testPath("/test/path.txt");
    slot.SetData(testPath);
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<std::filesystem::path>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), testPath);
}

TEST_F(SlotTest, SetAndGetMat)
{
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    slot.SetData(testMat);
    EXPECT_TRUE(slot.HasData());

    auto value = slot.GetData<cv::Mat>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value->rows, 100);
    EXPECT_EQ(value->cols, 100);
}

// ============================================================================
// Type Checking Tests
// ============================================================================

TEST_F(SlotTest, WrongTypeReturnsNullopt)
{
    slot.SetData(42);

    EXPECT_FALSE(slot.GetData<double>().has_value());
    EXPECT_FALSE(slot.GetData<std::string>().has_value());
    EXPECT_FALSE(slot.GetData<bool>().has_value());
    EXPECT_TRUE(slot.GetData<int>().has_value());
}

TEST_F(SlotTest, HoldsType)
{
    slot.SetData(3.14);

    EXPECT_TRUE(slot.HoldsType<double>());
    EXPECT_FALSE(slot.HoldsType<int>());
    EXPECT_FALSE(slot.HoldsType<std::string>());
    EXPECT_FALSE(slot.HoldsType<bool>());
}

TEST_F(SlotTest, TypeIndex)
{
    Slot emptySlot;
    EXPECT_EQ(emptySlot.GetTypeIndex(), 0); // monostate

    emptySlot.SetData(cv::Mat());
    EXPECT_EQ(emptySlot.GetTypeIndex(), 1); // cv::Mat

    emptySlot.SetData(3.14);
    EXPECT_EQ(emptySlot.GetTypeIndex(), 2); // double
}

// ============================================================================
// Clear and Reset Tests
// ============================================================================

TEST_F(SlotTest, ClearData)
{
    slot.SetData(42);
    EXPECT_TRUE(slot.HasData());

    slot.Clear();
    EXPECT_FALSE(slot.HasData());
    EXPECT_EQ(slot.GetTypeIndex(), 0);
    EXPECT_FALSE(slot.GetData<int>().has_value());
}

TEST_F(SlotTest, ClearDoesNotRemoveDefault)
{
    Slot slotWithDefault(NodeData(100));
    slotWithDefault.SetData(200);

    EXPECT_TRUE(slotWithDefault.HasData());
    EXPECT_TRUE(slotWithDefault.HasDefaultValue());

    slotWithDefault.Clear();

    EXPECT_FALSE(slotWithDefault.HasData());
    EXPECT_TRUE(slotWithDefault.HasDefaultValue());

    auto defaultVal = slotWithDefault.GetDefaultValue<int>();
    ASSERT_TRUE(defaultVal.has_value());
    EXPECT_EQ(defaultVal.value(), 100);
}

// ============================================================================
// Default Value Tests
// ============================================================================

TEST_F(SlotTest, SetDefaultValue)
{
    EXPECT_FALSE(slot.HasDefaultValue());

    slot.SetDefaultValue(NodeData(42));
    EXPECT_TRUE(slot.HasDefaultValue());

    auto defaultVal = slot.GetDefaultValue<int>();
    ASSERT_TRUE(defaultVal.has_value());
    EXPECT_EQ(defaultVal.value(), 42);
}

TEST_F(SlotTest, GetDefaultWrongType)
{
    slot.SetDefaultValue(NodeData(42));

    EXPECT_FALSE(slot.GetDefaultValue<double>().has_value());
    EXPECT_FALSE(slot.GetDefaultValue<std::string>().has_value());
    EXPECT_TRUE(slot.GetDefaultValue<int>().has_value());
}

TEST_F(SlotTest, OverwriteDefaultValue)
{
    slot.SetDefaultValue(NodeData(100));
    EXPECT_EQ(slot.GetDefaultValue<int>().value(), 100);

    slot.SetDefaultValue(NodeData(200));
    EXPECT_EQ(slot.GetDefaultValue<int>().value(), 200);
}

// ============================================================================
// GetValueOrDefault Tests
// ============================================================================

TEST_F(SlotTest, GetValueOrDefaultPrefersConnected)
{
    Slot slotWithDefault(NodeData(100));
    slotWithDefault.SetData(200);

    auto value = slotWithDefault.GetValueOrDefault<int>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 200); // Connected data wins
}

TEST_F(SlotTest, GetValueOrDefaultFallsBackToDefault)
{
    Slot slotWithDefault(NodeData(100));

    auto value = slotWithDefault.GetValueOrDefault<int>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 100); // Falls back to default
}

TEST_F(SlotTest, GetValueOrDefaultNoDataNoDefault)
{
    auto value = slot.GetValueOrDefault<int>();
    EXPECT_FALSE(value.has_value());
}

TEST_F(SlotTest, GetValueOrDefaultWrongType)
{
    Slot slotWithDefault(NodeData(42));
    slotWithDefault.SetData(3.14);

    // Try to get int when slot has double
    auto value = slotWithDefault.GetValueOrDefault<int>();
    // Should fall back to default since connected data type doesn't match
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 42);
}

// ============================================================================
// Connection State Tests
// ============================================================================

TEST_F(SlotTest, IsConnected)
{
    EXPECT_FALSE(slot.IsConnected());

    slot.SetData(42);
    EXPECT_TRUE(slot.IsConnected());

    slot.Clear();
    EXPECT_FALSE(slot.IsConnected());
}

TEST_F(SlotTest, IsConnectedIndependentOfDefault)
{
    Slot slotWithDefault(NodeData(100));

    EXPECT_FALSE(slotWithDefault.IsConnected());
    EXPECT_TRUE(slotWithDefault.HasDefaultValue());

    slotWithDefault.SetData(200);
    EXPECT_TRUE(slotWithDefault.IsConnected());
    EXPECT_TRUE(slotWithDefault.HasDefaultValue());
}

// ============================================================================
// Data Overwriting Tests
// ============================================================================

TEST_F(SlotTest, OverwriteData)
{
    slot.SetData(100);
    EXPECT_EQ(slot.GetData<int>().value(), 100);

    slot.SetData(200);
    EXPECT_EQ(slot.GetData<int>().value(), 200);
}

TEST_F(SlotTest, OverwriteWithDifferentType)
{
    slot.SetData(42);
    EXPECT_TRUE(slot.HoldsType<int>());

    slot.SetData(std::string("hello"));
    EXPECT_TRUE(slot.HoldsType<std::string>());
    EXPECT_FALSE(slot.HoldsType<int>());

    EXPECT_FALSE(slot.GetData<int>().has_value());
    EXPECT_EQ(slot.GetData<std::string>().value(), "hello");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(SlotTest, EmptyStringDefault)
{
    Slot slotWithEmpty(NodeData(std::string("")));
    EXPECT_TRUE(slotWithEmpty.HasDefaultValue());

    auto value = slotWithEmpty.GetDefaultValue<std::string>();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "");
}

TEST_F(SlotTest, ZeroValues)
{
    Slot intSlot(NodeData(0));
    EXPECT_EQ(intSlot.GetDefaultValue<int>().value(), 0);

    Slot doubleSlot(NodeData(0.0));
    EXPECT_EQ(doubleSlot.GetDefaultValue<double>().value(), 0.0);

    Slot boolSlot(NodeData(false));
    EXPECT_FALSE(boolSlot.GetDefaultValue<bool>().value());
}

TEST_F(SlotTest, EmptyMat)
{
    cv::Mat emptyMat;
    slot.SetData(emptyMat);

    EXPECT_TRUE(slot.HasData());
    auto retrieved = slot.GetData<cv::Mat>();
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_TRUE(retrieved->empty());
}
