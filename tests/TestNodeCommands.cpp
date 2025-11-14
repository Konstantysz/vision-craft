#include "Editor/Commands/NodeCommands.h"
#include "UI/Widgets/NodeEditorTypes.h"
#include "Nodes/Core/Node.h"
#include "Nodes/Core/NodeEditor.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <unordered_map>

using namespace VisionCraft;
using namespace VisionCraft::Editor::Commands;
using namespace VisionCraft::UI::Widgets;

// ============================================================================
// Mock Node for Testing
// ============================================================================

class MockNode : public Nodes::Node
{
public:
    MockNode(Nodes::NodeId id, const std::string &name, const std::string &type) : Node(id, name), nodeType(type)
    {
    }

    [[nodiscard]] std::string GetType() const override
    {
        return nodeType;
    }

    void Process() override
    {
        processCallCount++;
    }

    int GetProcessCallCount() const
    {
        return processCallCount;
    }

private:
    std::string nodeType;
    int processCallCount = 0;
};

// ============================================================================
// Test Fixture with Mock Node Management
// ============================================================================

class NodeCommandsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        nodeStorage.clear();
        nodePositions.clear();
        nextNodeId = 1;
    }

    // Helper: Create a mock node
    std::unique_ptr<Nodes::Node> CreateMockNode(Nodes::NodeId id, const std::string &name, const std::string &type)
    {
        return std::make_unique<MockNode>(id, name, type);
    }

    // Helper: Add node to storage
    void AddNode(std::unique_ptr<Nodes::Node> node)
    {
        nodeStorage[node->GetId()] = std::move(node);
    }

    // Helper: Remove node from storage
    void RemoveNode(Nodes::NodeId id)
    {
        nodeStorage.erase(id);
        nodePositions.erase(id);
    }

    // Helper: Get node pointer
    Nodes::Node *GetNode(Nodes::NodeId id)
    {
        auto it = nodeStorage.find(id);
        return it != nodeStorage.end() ? it->second.get() : nullptr;
    }

    // Helper: Set node position
    void SetNodePosition(Nodes::NodeId id, const NodePosition &pos)
    {
        nodePositions[id] = pos;
    }

    // Helper: Get node position
    NodePosition GetNodePosition(Nodes::NodeId id)
    {
        auto it = nodePositions.find(id);
        return it != nodePositions.end() ? it->second : NodePosition{ 0, 0 };
    }

    // Helper: Recreate node from type, id, and name
    std::unique_ptr<Nodes::Node> RecreateNode(const std::string &type, Nodes::NodeId id, const std::string &name)
    {
        return std::make_unique<MockNode>(id, name, type);
    }

    // Storage
    std::unordered_map<Nodes::NodeId, std::unique_ptr<Nodes::Node>> nodeStorage;
    std::unordered_map<Nodes::NodeId, NodePosition> nodePositions;
    Nodes::NodeId nextNodeId = 1;
};

// ============================================================================
// CreateNodeCommand Tests
// ============================================================================

TEST_F(NodeCommandsTest, CreateNodeCommand_Execute)
{
    NodePosition targetPos{ 100.0f, 200.0f };
    auto cmd =
        std::make_unique<CreateNodeCommand>([this]() { return CreateMockNode(nextNodeId++, "Test Node", "MockNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            targetPos,
            "MockNode");

    cmd->Execute();

    EXPECT_EQ(nodeStorage.size(), 1);
    EXPECT_NE(GetNode(1), nullptr);
    EXPECT_EQ(GetNode(1)->GetName(), "Test Node");
    EXPECT_EQ(nodePositions[1].x, 100.0f);
    EXPECT_EQ(nodePositions[1].y, 200.0f);
}

TEST_F(NodeCommandsTest, CreateNodeCommand_Undo)
{
    NodePosition targetPos{ 100.0f, 200.0f };
    auto cmd =
        std::make_unique<CreateNodeCommand>([this]() { return CreateMockNode(nextNodeId++, "Test Node", "MockNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            targetPos,
            "MockNode");

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 1);

    cmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 0);
    EXPECT_EQ(nodePositions.size(), 0);
}

TEST_F(NodeCommandsTest, CreateNodeCommand_ExecuteUndoExecute)
{
    NodePosition targetPos{ 50.0f, 50.0f };
    auto cmd =
        std::make_unique<CreateNodeCommand>([this]() { return CreateMockNode(nextNodeId++, "Node A", "MockNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            targetPos,
            "MockNode");

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 1);

    cmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 0);

    // Note: Re-executing creates a NEW node with a new ID
    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 1);
    EXPECT_NE(GetNode(2), nullptr); // New ID = 2
}

TEST_F(NodeCommandsTest, CreateNodeCommand_GetDescription)
{
    NodePosition targetPos{ 0, 0 };
    auto cmd =
        std::make_unique<CreateNodeCommand>([this]() { return CreateMockNode(nextNodeId++, "Test", "GrayscaleNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            targetPos,
            "GrayscaleNode");

    EXPECT_EQ(cmd->GetDescription(), "Create GrayscaleNode");
}

TEST_F(NodeCommandsTest, CreateNodeCommand_MultipleNodes)
{
    std::vector<std::unique_ptr<CreateNodeCommand>> commands;

    for (int i = 0; i < 5; ++i)
    {
        NodePosition pos{ static_cast<float>(i * 100), static_cast<float>(i * 50) };
        commands.push_back(std::make_unique<CreateNodeCommand>(
            [this, i]() { return CreateMockNode(nextNodeId++, "Node " + std::to_string(i), "MockNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            pos,
            "MockNode"));
    }

    for (auto &cmd : commands)
    {
        cmd->Execute();
    }

    EXPECT_EQ(nodeStorage.size(), 5);

    for (auto &cmd : commands)
    {
        cmd->Undo();
    }

    EXPECT_EQ(nodeStorage.size(), 0);
}

// ============================================================================
// DeleteNodeCommand Tests
// ============================================================================

TEST_F(NodeCommandsTest, DeleteNodeCommand_Execute)
{
    // Setup: Create a node
    AddNode(CreateMockNode(1, "Node to Delete", "MockNode"));
    SetNodePosition(1, { 100.0f, 200.0f });
    EXPECT_EQ(nodeStorage.size(), 1);

    auto cmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    cmd->Execute();

    EXPECT_EQ(nodeStorage.size(), 0);
}

TEST_F(NodeCommandsTest, DeleteNodeCommand_Undo)
{
    // Setup: Create a node
    AddNode(CreateMockNode(1, "Recoverable Node", "MockNode"));
    SetNodePosition(1, { 150.0f, 250.0f });

    auto cmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);

    cmd->Undo();

    EXPECT_EQ(nodeStorage.size(), 1);
    EXPECT_NE(GetNode(1), nullptr);
    EXPECT_EQ(GetNode(1)->GetName(), "Recoverable Node");
    EXPECT_EQ(GetNode(1)->GetType(), "MockNode");
    EXPECT_EQ(nodePositions[1].x, 150.0f);
    EXPECT_EQ(nodePositions[1].y, 250.0f);
}

TEST_F(NodeCommandsTest, DeleteNodeCommand_ExecuteUndoExecute)
{
    AddNode(CreateMockNode(1, "Test Node", "MockNode"));
    SetNodePosition(1, { 50.0f, 50.0f });

    auto cmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);

    cmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 1);

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);
}

TEST_F(NodeCommandsTest, DeleteNodeCommand_GetDescription)
{
    AddNode(CreateMockNode(1, "Special Node", "MockNode"));

    auto cmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    cmd->Execute();

    EXPECT_EQ(cmd->GetDescription(), "Delete Special Node");
}

TEST_F(NodeCommandsTest, DeleteNodeCommand_NonExistentNode)
{
    // Deleting a node that doesn't exist should not crash
    auto cmd = std::make_unique<DeleteNodeCommand>(
        999,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    cmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);

    // Undo should not crash
    cmd->Undo();
}

// ============================================================================
// MoveNodesCommand Tests
// ============================================================================

TEST_F(NodeCommandsTest, MoveNodesCommand_SingleNode)
{
    AddNode(CreateMockNode(1, "Movable Node", "MockNode"));
    SetNodePosition(1, { 0.0f, 0.0f });

    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{ { 1, { 0.0f, 0.0f } } };
    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{ { 1, { 100.0f, 200.0f } } };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    cmd->Execute();

    EXPECT_EQ(nodePositions[1].x, 100.0f);
    EXPECT_EQ(nodePositions[1].y, 200.0f);
}

TEST_F(NodeCommandsTest, MoveNodesCommand_Undo)
{
    AddNode(CreateMockNode(1, "Node", "MockNode"));
    SetNodePosition(1, { 50.0f, 50.0f });

    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{ { 1, { 50.0f, 50.0f } } };
    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{ { 1, { 150.0f, 250.0f } } };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    cmd->Execute();
    EXPECT_EQ(nodePositions[1].x, 150.0f);
    EXPECT_EQ(nodePositions[1].y, 250.0f);

    cmd->Undo();
    EXPECT_EQ(nodePositions[1].x, 50.0f);
    EXPECT_EQ(nodePositions[1].y, 50.0f);
}

TEST_F(NodeCommandsTest, MoveNodesCommand_MultipleNodes)
{
    AddNode(CreateMockNode(1, "Node 1", "MockNode"));
    AddNode(CreateMockNode(2, "Node 2", "MockNode"));
    AddNode(CreateMockNode(3, "Node 3", "MockNode"));

    SetNodePosition(1, { 0.0f, 0.0f });
    SetNodePosition(2, { 100.0f, 0.0f });
    SetNodePosition(3, { 200.0f, 0.0f });

    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{
        { 1, { 0.0f, 0.0f } }, { 2, { 100.0f, 0.0f } }, { 3, { 200.0f, 0.0f } }
    };

    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{
        { 1, { 50.0f, 50.0f } }, { 2, { 150.0f, 50.0f } }, { 3, { 250.0f, 50.0f } }
    };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    cmd->Execute();

    EXPECT_EQ(nodePositions[1].x, 50.0f);
    EXPECT_EQ(nodePositions[2].x, 150.0f);
    EXPECT_EQ(nodePositions[3].x, 250.0f);

    cmd->Undo();

    EXPECT_EQ(nodePositions[1].x, 0.0f);
    EXPECT_EQ(nodePositions[2].x, 100.0f);
    EXPECT_EQ(nodePositions[3].x, 200.0f);
}

TEST_F(NodeCommandsTest, MoveNodesCommand_GetDescription_SingleNode)
{
    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{ { 1, { 0.0f, 0.0f } } };
    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{ { 1, { 100.0f, 100.0f } } };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    EXPECT_EQ(cmd->GetDescription(), "Move Node");
}

TEST_F(NodeCommandsTest, MoveNodesCommand_GetDescription_MultipleNodes)
{
    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{
        { 1, { 0.0f, 0.0f } }, { 2, { 0.0f, 0.0f } }, { 3, { 0.0f, 0.0f } }
    };
    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{
        { 1, { 10.0f, 10.0f } }, { 2, { 20.0f, 20.0f } }, { 3, { 30.0f, 30.0f } }
    };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    EXPECT_EQ(cmd->GetDescription(), "Move 3 Nodes");
}

TEST_F(NodeCommandsTest, MoveNodesCommand_EmptyMove)
{
    std::unordered_map<Nodes::NodeId, NodePosition> emptyMap;

    auto cmd = std::make_unique<MoveNodesCommand>(
        emptyMap, emptyMap, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    cmd->Execute();
    cmd->Undo();

    // Should not crash
    EXPECT_EQ(nodePositions.size(), 0);
}

TEST_F(NodeCommandsTest, MoveNodesCommand_RepeatedExecuteUndo)
{
    AddNode(CreateMockNode(1, "Node", "MockNode"));
    SetNodePosition(1, { 0.0f, 0.0f });

    std::unordered_map<Nodes::NodeId, NodePosition> oldPositions{ { 1, { 0.0f, 0.0f } } };
    std::unordered_map<Nodes::NodeId, NodePosition> newPositions{ { 1, { 100.0f, 100.0f } } };

    auto cmd = std::make_unique<MoveNodesCommand>(
        oldPositions, newPositions, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    for (int i = 0; i < 10; ++i)
    {
        cmd->Execute();
        EXPECT_EQ(nodePositions[1].x, 100.0f);
        EXPECT_EQ(nodePositions[1].y, 100.0f);

        cmd->Undo();
        EXPECT_EQ(nodePositions[1].x, 0.0f);
        EXPECT_EQ(nodePositions[1].y, 0.0f);
    }
}

// ============================================================================
// Integration Tests - Combining Commands
// ============================================================================

TEST_F(NodeCommandsTest, Integration_CreateThenDelete)
{
    // Create a node
    NodePosition pos{ 100.0f, 100.0f };
    auto createCmd = std::make_unique<CreateNodeCommand>(
        [this]() { return CreateMockNode(nextNodeId++, "Temporary Node", "MockNode"); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        pos,
        "MockNode");

    createCmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 1);

    // Delete the node
    auto deleteCmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    deleteCmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);

    // Undo delete
    deleteCmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 1);

    // Undo create
    createCmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 0);
}

TEST_F(NodeCommandsTest, Integration_CreateMoveDelete)
{
    // Create
    NodePosition initialPos{ 0.0f, 0.0f };
    auto createCmd =
        std::make_unique<CreateNodeCommand>([this]() { return CreateMockNode(nextNodeId++, "Node", "MockNode"); },
            [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
            [this](Nodes::NodeId id) { RemoveNode(id); },
            [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
            initialPos,
            "MockNode");

    createCmd->Execute();

    // Move
    std::unordered_map<Nodes::NodeId, NodePosition> oldPos{ { 1, { 0.0f, 0.0f } } };
    std::unordered_map<Nodes::NodeId, NodePosition> newPos{ { 1, { 200.0f, 200.0f } } };
    auto moveCmd = std::make_unique<MoveNodesCommand>(
        oldPos, newPos, [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); });

    moveCmd->Execute();
    EXPECT_EQ(nodePositions[1].x, 200.0f);

    // Delete
    auto deleteCmd = std::make_unique<DeleteNodeCommand>(
        1,
        [this](Nodes::NodeId id) { return GetNode(id); },
        [this](Nodes::NodeId id) { RemoveNode(id); },
        [this](std::unique_ptr<Nodes::Node> node) { AddNode(std::move(node)); },
        [this](Nodes::NodeId id) { return GetNodePosition(id); },
        [this](Nodes::NodeId id, const NodePosition &pos) { SetNodePosition(id, pos); },
        [this](const std::string &type, Nodes::NodeId id, const std::string &name) {
            return RecreateNode(type, id, name);
        });

    deleteCmd->Execute();
    EXPECT_EQ(nodeStorage.size(), 0);

    // Undo in reverse order
    deleteCmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 1);
    EXPECT_EQ(nodePositions[1].x, 200.0f); // Position preserved after delete undo

    moveCmd->Undo();
    EXPECT_EQ(nodePositions[1].x, 0.0f);

    createCmd->Undo();
    EXPECT_EQ(nodeStorage.size(), 0);
}
