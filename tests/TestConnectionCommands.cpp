#include "Editor/Commands/ConnectionCommands.h"
#include "UI/Widgets/NodeEditorTypes.h"

#include <gtest/gtest.h>
#include <memory>
#include <set>
#include <string>

using namespace VisionCraft::Editor::Commands;
using namespace VisionCraft::UI::Widgets;

// ============================================================================
// Test Fixture with Connection Management
// ============================================================================

class ConnectionCommandsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        connections.clear();
    }

    // Helper: Create a connection
    void CreateConnection(const PinId &outputPin, const PinId &inputPin)
    {
        NodeConnection conn;
        conn.outputPin = outputPin;
        conn.inputPin = inputPin;
        connections.insert(conn);
    }

    // Helper: Remove a connection
    void RemoveConnection(const NodeConnection &connection)
    {
        connections.erase(connection);
    }

    // Helper: Check if connection exists
    bool ConnectionExists(const PinId &outputPin, const PinId &inputPin) const
    {
        NodeConnection conn;
        conn.outputPin = outputPin;
        conn.inputPin = inputPin;
        return connections.find(conn) != connections.end();
    }

    // Storage for connections
    struct ConnectionComparator
    {
        bool operator()(const NodeConnection &a, const NodeConnection &b) const
        {
            if (a.outputPin.nodeId != b.outputPin.nodeId)
                return a.outputPin.nodeId < b.outputPin.nodeId;
            if (a.outputPin.pinName != b.outputPin.pinName)
                return a.outputPin.pinName < b.outputPin.pinName;
            if (a.inputPin.nodeId != b.inputPin.nodeId)
                return a.inputPin.nodeId < b.inputPin.nodeId;
            return a.inputPin.pinName < b.inputPin.pinName;
        }
    };

    std::set<NodeConnection, ConnectionComparator> connections;
};

// ============================================================================
// CreateConnectionCommand Tests
// ============================================================================

TEST_F(ConnectionCommandsTest, CreateConnectionCommand_Execute)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    cmd->Execute();

    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));
    EXPECT_EQ(connections.size(), 1);
}

TEST_F(ConnectionCommandsTest, CreateConnectionCommand_Undo)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    cmd->Execute();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    cmd->Undo();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
    EXPECT_EQ(connections.size(), 0);
}

TEST_F(ConnectionCommandsTest, CreateConnectionCommand_ExecuteUndoExecute)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    cmd->Execute();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    cmd->Undo();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));

    cmd->Execute();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));
}

TEST_F(ConnectionCommandsTest, CreateConnectionCommand_GetDescription)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    EXPECT_EQ(cmd->GetDescription(), "Create Connection");
}

TEST_F(ConnectionCommandsTest, CreateConnectionCommand_MultipleDifferentConnections)
{
    PinId output1{ 1, "Out1" };
    PinId output2{ 2, "Out2" };
    PinId input1{ 3, "In1" };
    PinId input2{ 4, "In2" };

    auto cmd1 = std::make_unique<CreateConnectionCommand>(
        output1,
        input1,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    auto cmd2 = std::make_unique<CreateConnectionCommand>(
        output2,
        input2,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    cmd1->Execute();
    cmd2->Execute();

    EXPECT_EQ(connections.size(), 2);
    EXPECT_TRUE(ConnectionExists(output1, input1));
    EXPECT_TRUE(ConnectionExists(output2, input2));

    cmd1->Undo();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_FALSE(ConnectionExists(output1, input1));
    EXPECT_TRUE(ConnectionExists(output2, input2));

    cmd2->Undo();
    EXPECT_EQ(connections.size(), 0);
}

// ============================================================================
// DeleteConnectionCommand Tests
// ============================================================================

TEST_F(ConnectionCommandsTest, DeleteConnectionCommand_Execute)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    // Setup: Create a connection first
    CreateConnection(outputPin, inputPin);
    EXPECT_EQ(connections.size(), 1);

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto cmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    cmd->Execute();

    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
    EXPECT_EQ(connections.size(), 0);
}

TEST_F(ConnectionCommandsTest, DeleteConnectionCommand_Undo)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    CreateConnection(outputPin, inputPin);

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto cmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    cmd->Execute();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));

    cmd->Undo();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));
    EXPECT_EQ(connections.size(), 1);
}

TEST_F(ConnectionCommandsTest, DeleteConnectionCommand_ExecuteUndoExecute)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    CreateConnection(outputPin, inputPin);

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto cmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    cmd->Execute();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));

    cmd->Undo();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    cmd->Execute();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
}

TEST_F(ConnectionCommandsTest, DeleteConnectionCommand_GetDescription)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto cmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    EXPECT_EQ(cmd->GetDescription(), "Delete Connection");
}

TEST_F(ConnectionCommandsTest, DeleteConnectionCommand_NonExistentConnection)
{
    PinId outputPin{ 999, "NonExistent" };
    PinId inputPin{ 1000, "NonExistent" };

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto cmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    // Should not crash even if connection doesn't exist
    cmd->Execute();
    EXPECT_EQ(connections.size(), 0);

    // Undo should recreate it
    cmd->Undo();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));
}

// ============================================================================
// Integration Tests - Create and Delete
// ============================================================================

TEST_F(ConnectionCommandsTest, Integration_CreateThenDelete)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto createCmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    createCmd->Execute();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    NodeConnection conn;
    conn.outputPin = outputPin;
    conn.inputPin = inputPin;

    auto deleteCmd = std::make_unique<DeleteConnectionCommand>(
        conn,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    deleteCmd->Execute();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));

    // Undo in reverse order
    deleteCmd->Undo();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    createCmd->Undo();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
}

TEST_F(ConnectionCommandsTest, Integration_MultipleConnectionsCreateDelete)
{
    std::vector<std::unique_ptr<CreateConnectionCommand>> createCommands;

    for (int i = 0; i < 5; ++i)
    {
        PinId output{ i, "Output" };
        PinId input{ i + 100, "Input" };

        createCommands.push_back(std::make_unique<CreateConnectionCommand>(
            output,
            input,
            [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
            [this](const NodeConnection &conn) { RemoveConnection(conn); }));
    }

    // Create all connections
    for (auto &cmd : createCommands)
    {
        cmd->Execute();
    }
    EXPECT_EQ(connections.size(), 5);

    // Undo all
    for (auto &cmd : createCommands)
    {
        cmd->Undo();
    }
    EXPECT_EQ(connections.size(), 0);

    // Redo all
    for (auto &cmd : createCommands)
    {
        cmd->Execute();
    }
    EXPECT_EQ(connections.size(), 5);
}

TEST_F(ConnectionCommandsTest, Integration_AlternatingCreateDelete)
{
    PinId output1{ 1, "Out1" };
    PinId input1{ 2, "In1" };
    PinId output2{ 3, "Out2" };
    PinId input2{ 4, "In2" };

    auto createCmd1 = std::make_unique<CreateConnectionCommand>(
        output1,
        input1,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    auto createCmd2 = std::make_unique<CreateConnectionCommand>(
        output2,
        input2,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    createCmd1->Execute();
    EXPECT_EQ(connections.size(), 1);

    createCmd2->Execute();
    EXPECT_EQ(connections.size(), 2);

    NodeConnection conn1{ output1, input1 };
    auto deleteCmd1 = std::make_unique<DeleteConnectionCommand>(
        conn1,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &c) { RemoveConnection(c); });

    deleteCmd1->Execute();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_TRUE(ConnectionExists(output2, input2));

    deleteCmd1->Undo();
    EXPECT_EQ(connections.size(), 2);

    createCmd1->Undo();
    EXPECT_EQ(connections.size(), 1);
    EXPECT_TRUE(ConnectionExists(output2, input2));

    createCmd2->Undo();
    EXPECT_EQ(connections.size(), 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ConnectionCommandsTest, EdgeCase_RepeatedExecuteUndo)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 2, "Input" };

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    for (int i = 0; i < 10; ++i)
    {
        cmd->Execute();
        EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

        cmd->Undo();
        EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
    }
}

TEST_F(ConnectionCommandsTest, EdgeCase_SameNodeConnection)
{
    PinId outputPin{ 1, "Output" };
    PinId inputPin{ 1, "Input" }; // Same node ID

    auto cmd = std::make_unique<CreateConnectionCommand>(
        outputPin,
        inputPin,
        [this](const PinId &out, const PinId &in) { CreateConnection(out, in); },
        [this](const NodeConnection &conn) { RemoveConnection(conn); });

    cmd->Execute();
    EXPECT_TRUE(ConnectionExists(outputPin, inputPin));

    cmd->Undo();
    EXPECT_FALSE(ConnectionExists(outputPin, inputPin));
}
