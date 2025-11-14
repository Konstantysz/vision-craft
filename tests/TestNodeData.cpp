#include "Nodes/Core/NodeData.h"

#include <opencv2/opencv.hpp>
#include <gtest/gtest.h>
#include <filesystem>

using namespace VisionCraft;

class NodeDataTest : public ::testing::Test
{
protected:
    // Helper functions for testing
    template<typename T> bool HoldsType(const Nodes::NodeData &data)
    {
        return std::holds_alternative<T>(data);
    }
};

// ============================================================================
// Basic Variant Construction Tests
// ============================================================================

TEST_F(NodeDataTest, DefaultConstruction)
{
    Nodes::NodeData data;
    EXPECT_TRUE(HoldsType<std::monostate>(data));
}

TEST_F(NodeDataTest, ConstructWithInt)
{
    Nodes::NodeData data = 42;
    EXPECT_TRUE(HoldsType<int>(data));
    EXPECT_EQ(std::get<int>(data), 42);
}

TEST_F(NodeDataTest, ConstructWithDouble)
{
    Nodes::NodeData data = 3.14159;
    EXPECT_TRUE(HoldsType<double>(data));
    EXPECT_DOUBLE_EQ(std::get<double>(data), 3.14159);
}

TEST_F(NodeDataTest, ConstructWithFloat)
{
    Nodes::NodeData data = 2.71828f;
    EXPECT_TRUE(HoldsType<float>(data));
    EXPECT_FLOAT_EQ(std::get<float>(data), 2.71828f);
}

TEST_F(NodeDataTest, ConstructWithBool)
{
    Nodes::NodeData dataTrue = true;
    Nodes::NodeData dataFalse = false;
    EXPECT_TRUE(HoldsType<bool>(dataTrue));
    EXPECT_TRUE(HoldsType<bool>(dataFalse));
    EXPECT_TRUE(std::get<bool>(dataTrue));
    EXPECT_FALSE(std::get<bool>(dataFalse));
}

TEST_F(NodeDataTest, ConstructWithString)
{
    Nodes::NodeData data = std::string("test_string");
    EXPECT_TRUE(HoldsType<std::string>(data));
    EXPECT_EQ(std::get<std::string>(data), "test_string");
}

TEST_F(NodeDataTest, ConstructWithPath)
{
    std::filesystem::path testPath("/test/path.txt");
    Nodes::NodeData data = testPath;
    EXPECT_TRUE(HoldsType<std::filesystem::path>(data));
    EXPECT_EQ(std::get<std::filesystem::path>(data), testPath);
}

TEST_F(NodeDataTest, ConstructWithMat)
{
    cv::Mat testMat = cv::Mat::zeros(100, 100, CV_8UC3);
    Nodes::NodeData data = testMat;
    EXPECT_TRUE(HoldsType<cv::Mat>(data));

    const auto &mat = std::get<cv::Mat>(data);
    EXPECT_EQ(mat.rows, 100);
    EXPECT_EQ(mat.cols, 100);
    EXPECT_EQ(mat.type(), CV_8UC3);
}

TEST_F(NodeDataTest, ConstructWithVectorPoints)
{
    std::vector<cv::Point> points = { { 0, 0 }, { 10, 10 }, { 20, 20 } };
    Nodes::NodeData data = points;
    EXPECT_TRUE(HoldsType<std::vector<cv::Point>>(data));

    const auto &retrievedPoints = std::get<std::vector<cv::Point>>(data);
    EXPECT_EQ(retrievedPoints.size(), 3);
    EXPECT_EQ(retrievedPoints[0], cv::Point(0, 0));
    EXPECT_EQ(retrievedPoints[1], cv::Point(10, 10));
    EXPECT_EQ(retrievedPoints[2], cv::Point(20, 20));
}

// ============================================================================
// Type Checking Tests
// ============================================================================

TEST_F(NodeDataTest, TypeIndex)
{
    Nodes::NodeData emptyData;
    EXPECT_EQ(emptyData.index(), 0); // monostate

    Nodes::NodeData matData = cv::Mat();
    EXPECT_EQ(matData.index(), 1); // cv::Mat

    Nodes::NodeData doubleData = 3.14;
    EXPECT_EQ(doubleData.index(), 2); // double

    Nodes::NodeData floatData = 2.71f;
    EXPECT_EQ(floatData.index(), 3); // float

    Nodes::NodeData intData = 42;
    EXPECT_EQ(intData.index(), 4); // int

    Nodes::NodeData boolData = true;
    EXPECT_EQ(boolData.index(), 5); // bool

    Nodes::NodeData stringData = std::string("test");
    EXPECT_EQ(stringData.index(), 6); // string

    Nodes::NodeData pathData = std::filesystem::path("/test");
    EXPECT_EQ(pathData.index(), 7); // path

    Nodes::NodeData pointsData = std::vector<cv::Point>{};
    EXPECT_EQ(pointsData.index(), 8); // vector<Point>
}

TEST_F(NodeDataTest, HoldsAlternative)
{
    Nodes::NodeData data = 42;

    EXPECT_TRUE(std::holds_alternative<int>(data));
    EXPECT_FALSE(std::holds_alternative<double>(data));
    EXPECT_FALSE(std::holds_alternative<float>(data));
    EXPECT_FALSE(std::holds_alternative<bool>(data));
    EXPECT_FALSE(std::holds_alternative<std::string>(data));
    EXPECT_FALSE(std::holds_alternative<cv::Mat>(data));
    EXPECT_FALSE(std::holds_alternative<std::filesystem::path>(data));
    EXPECT_FALSE(std::holds_alternative<std::vector<cv::Point>>(data));
}

// ============================================================================
// Assignment and Copy Tests
// ============================================================================

TEST_F(NodeDataTest, CopyConstruction)
{
    Nodes::NodeData original = 42;
    Nodes::NodeData copy = original;

    EXPECT_TRUE(HoldsType<int>(copy));
    EXPECT_EQ(std::get<int>(copy), 42);

    // Modify copy shouldn't affect original
    copy = 100;
    EXPECT_EQ(std::get<int>(original), 42);
    EXPECT_EQ(std::get<int>(copy), 100);
}

TEST_F(NodeDataTest, CopyAssignment)
{
    Nodes::NodeData original = std::string("hello");
    Nodes::NodeData copy;

    copy = original;

    EXPECT_TRUE(HoldsType<std::string>(copy));
    EXPECT_EQ(std::get<std::string>(copy), "hello");

    // Modify copy shouldn't affect original
    copy = std::string("world");
    EXPECT_EQ(std::get<std::string>(original), "hello");
    EXPECT_EQ(std::get<std::string>(copy), "world");
}

TEST_F(NodeDataTest, MoveConstruction)
{
    Nodes::NodeData original = std::string("temporary");
    Nodes::NodeData moved = std::move(original);

    EXPECT_TRUE(HoldsType<std::string>(moved));
    EXPECT_EQ(std::get<std::string>(moved), "temporary");
}

TEST_F(NodeDataTest, MoveAssignment)
{
    Nodes::NodeData original = std::string("temporary");
    Nodes::NodeData moved;

    moved = std::move(original);

    EXPECT_TRUE(HoldsType<std::string>(moved));
    EXPECT_EQ(std::get<std::string>(moved), "temporary");
}

TEST_F(NodeDataTest, ReassignDifferentTypes)
{
    Nodes::NodeData data = 42;
    EXPECT_TRUE(HoldsType<int>(data));
    EXPECT_EQ(std::get<int>(data), 42);

    data = 3.14;
    EXPECT_TRUE(HoldsType<double>(data));
    EXPECT_DOUBLE_EQ(std::get<double>(data), 3.14);

    data = std::string("test");
    EXPECT_TRUE(HoldsType<std::string>(data));
    EXPECT_EQ(std::get<std::string>(data), "test");
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

TEST_F(NodeDataTest, EmptyString)
{
    Nodes::NodeData data = std::string("");
    EXPECT_TRUE(HoldsType<std::string>(data));
    EXPECT_EQ(std::get<std::string>(data), "");
}

TEST_F(NodeDataTest, ZeroValues)
{
    Nodes::NodeData intData = 0;
    EXPECT_EQ(std::get<int>(intData), 0);

    Nodes::NodeData doubleData = 0.0;
    EXPECT_EQ(std::get<double>(doubleData), 0.0);

    Nodes::NodeData floatData = 0.0f;
    EXPECT_EQ(std::get<float>(floatData), 0.0f);

    Nodes::NodeData boolData = false;
    EXPECT_FALSE(std::get<bool>(boolData));
}

TEST_F(NodeDataTest, NegativeValues)
{
    Nodes::NodeData intData = -42;
    EXPECT_EQ(std::get<int>(intData), -42);

    Nodes::NodeData doubleData = -3.14;
    EXPECT_DOUBLE_EQ(std::get<double>(doubleData), -3.14);

    Nodes::NodeData floatData = -2.71f;
    EXPECT_FLOAT_EQ(std::get<float>(floatData), -2.71f);
}

TEST_F(NodeDataTest, EmptyMat)
{
    cv::Mat emptyMat;
    Nodes::NodeData data = emptyMat;

    EXPECT_TRUE(HoldsType<cv::Mat>(data));
    const auto &mat = std::get<cv::Mat>(data);
    EXPECT_TRUE(mat.empty());
}

TEST_F(NodeDataTest, EmptyVectorPoints)
{
    std::vector<cv::Point> emptyPoints;
    Nodes::NodeData data = emptyPoints;

    EXPECT_TRUE(HoldsType<std::vector<cv::Point>>(data));
    const auto &points = std::get<std::vector<cv::Point>>(data);
    EXPECT_TRUE(points.empty());
}

TEST_F(NodeDataTest, LargeString)
{
    std::string largeString(10000, 'a');
    Nodes::NodeData data = largeString;

    EXPECT_TRUE(HoldsType<std::string>(data));
    EXPECT_EQ(std::get<std::string>(data).size(), 10000);
}

TEST_F(NodeDataTest, ComplexPath)
{
    std::filesystem::path complexPath = std::filesystem::path("C:") / "Users" / "Test User" / "Documents" / "file.txt";
    Nodes::NodeData data = complexPath;

    EXPECT_TRUE(HoldsType<std::filesystem::path>(data));
    EXPECT_EQ(std::get<std::filesystem::path>(data), complexPath);
}

// ============================================================================
// Mat Deep Copy Tests
// ============================================================================

TEST_F(NodeDataTest, MatDeepCopy)
{
    cv::Mat original = cv::Mat::ones(10, 10, CV_8UC1) * 100;
    Nodes::NodeData data = original;

    // OpenCV Mat uses reference counting by default, so this will affect both
    // To test deep copy, we need to explicitly clone
    cv::Mat originalClone = original.clone();
    Nodes::NodeData dataFromClone = originalClone;

    // Modify original
    original.at<uchar>(5, 5) = 200;

    // Check that the clone's data is independent
    const auto &dataMat = std::get<cv::Mat>(dataFromClone);
    EXPECT_EQ(dataMat.at<uchar>(5, 5), 100);  // Should still be 100 (from clone)
    EXPECT_EQ(original.at<uchar>(5, 5), 200); // Original is modified
}

// ============================================================================
// Visitor Pattern Tests
// ============================================================================

TEST_F(NodeDataTest, VisitVariant)
{
    Nodes::NodeData data = 42;

    std::string result = std::visit(
        [](auto &&arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>)
            {
                return "int";
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return "double";
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return "string";
            }
            else if constexpr (std::is_same_v<T, cv::Mat>)
            {
                return "Mat";
            }
            else
            {
                return "other";
            }
        },
        data);

    EXPECT_EQ(result, "int");
}

TEST_F(NodeDataTest, VisitMultipleTypes)
{
    std::vector<Nodes::NodeData> dataList = { 42, 3.14, std::string("test"), cv::Mat::zeros(10, 10, CV_8UC1) };

    std::vector<std::string> types;
    for (const auto &data : dataList)
    {
        std::string type = std::visit(
            [](auto &&arg) -> std::string {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>)
                    return "int";
                else if constexpr (std::is_same_v<T, double>)
                    return "double";
                else if constexpr (std::is_same_v<T, std::string>)
                    return "string";
                else if constexpr (std::is_same_v<T, cv::Mat>)
                    return "Mat";
                else
                    return "other";
            },
            data);
        types.push_back(type);
    }

    EXPECT_EQ(types[0], "int");
    EXPECT_EQ(types[1], "double");
    EXPECT_EQ(types[2], "string");
    EXPECT_EQ(types[3], "Mat");
}

// ============================================================================
// std::get Exception Tests
// ============================================================================

TEST_F(NodeDataTest, GetWrongTypeThrows)
{
    Nodes::NodeData data = 42;

    EXPECT_THROW(std::get<double>(data), std::bad_variant_access);
    EXPECT_THROW(std::get<std::string>(data), std::bad_variant_access);
    EXPECT_THROW(std::get<cv::Mat>(data), std::bad_variant_access);
}

// ============================================================================
// std::get_if Tests
// ============================================================================

TEST_F(NodeDataTest, GetIfCorrectType)
{
    Nodes::NodeData data = 42;

    int *ptr = std::get_if<int>(&data);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

TEST_F(NodeDataTest, GetIfWrongType)
{
    Nodes::NodeData data = 42;

    double *ptr = std::get_if<double>(&data);
    EXPECT_EQ(ptr, nullptr);

    std::string *strPtr = std::get_if<std::string>(&data);
    EXPECT_EQ(strPtr, nullptr);
}
