#include "Editor/Commands/Command.h"
#include "Editor/Commands/CommandHistory.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace VisionCraft::Editor::Commands;

// ============================================================================
// Mock Command Classes for Testing
// ============================================================================

/**
 * @brief Simple mock command that increments/decrements a counter.
 */
class MockCommand : public Command
{
public:
    MockCommand(int &counter, int delta, const std::string &description)
        : counter(counter), delta(delta), description(description)
    {
    }

    void Execute() override
    {
        counter += delta;
        executeCount++;
    }

    void Undo() override
    {
        counter -= delta;
        undoCount++;
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return description;
    }

    int GetExecuteCount() const
    {
        return executeCount;
    }
    int GetUndoCount() const
    {
        return undoCount;
    }

private:
    int &counter;
    int delta;
    std::string description;
    int executeCount = 0;
    int undoCount = 0;
};

/**
 * @brief Mock command that modifies a string.
 */
class StringCommand : public Command
{
public:
    StringCommand(std::string &target, const std::string &newValue, const std::string &description)
        : target(target), newValue(newValue), oldValue(target), description(description)
    {
    }

    void Execute() override
    {
        target = newValue;
    }

    void Undo() override
    {
        target = oldValue;
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return description;
    }

private:
    std::string &target;
    std::string newValue;
    std::string oldValue;
    std::string description;
};

// ============================================================================
// CommandHistory Basic Functionality Tests
// ============================================================================

class CommandHistoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        counter = 0;
        history = std::make_unique<CommandHistory>();
    }

    int counter = 0;
    std::unique_ptr<CommandHistory> history;
};

TEST_F(CommandHistoryTest, DefaultConstruction)
{
    EXPECT_EQ(history->GetHistorySize(), 0);
    EXPECT_EQ(history->GetCurrentIndex(), 0);
    EXPECT_FALSE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, CustomMaxHistorySize)
{
    auto customHistory = std::make_unique<CommandHistory>(50);
    EXPECT_EQ(customHistory->GetHistorySize(), 0);
    EXPECT_EQ(customHistory->GetCurrentIndex(), 0);
}

TEST_F(CommandHistoryTest, ExecuteCommand)
{
    auto cmd = std::make_unique<MockCommand>(counter, 5, "Add 5");

    history->ExecuteCommand(std::move(cmd));

    EXPECT_EQ(counter, 5);
    EXPECT_EQ(history->GetHistorySize(), 1);
    EXPECT_EQ(history->GetCurrentIndex(), 1);
    EXPECT_TRUE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, ExecuteMultipleCommands)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, -2, "Subtract 2"));

    EXPECT_EQ(counter, 6); // 5 + 3 - 2
    EXPECT_EQ(history->GetHistorySize(), 3);
    EXPECT_EQ(history->GetCurrentIndex(), 3);
    EXPECT_TRUE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

// ============================================================================
// Undo/Redo Functionality Tests
// ============================================================================

TEST_F(CommandHistoryTest, UndoSingleCommand)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 10, "Add 10"));
    EXPECT_EQ(counter, 10);

    bool result = history->Undo();

    EXPECT_TRUE(result);
    EXPECT_EQ(counter, 0);
    EXPECT_EQ(history->GetCurrentIndex(), 0);
    EXPECT_FALSE(history->CanUndo());
    EXPECT_TRUE(history->CanRedo());
}

TEST_F(CommandHistoryTest, UndoMultipleCommands)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 2, "Add 2"));
    EXPECT_EQ(counter, 10);

    history->Undo();
    EXPECT_EQ(counter, 8); // Undid "Add 2"

    history->Undo();
    EXPECT_EQ(counter, 5); // Undid "Add 3"

    history->Undo();
    EXPECT_EQ(counter, 0); // Undid "Add 5"

    EXPECT_FALSE(history->CanUndo());
    EXPECT_TRUE(history->CanRedo());
}

TEST_F(CommandHistoryTest, UndoWhenEmpty)
{
    bool result = history->Undo();

    EXPECT_FALSE(result);
    EXPECT_EQ(counter, 0);
}

TEST_F(CommandHistoryTest, RedoSingleCommand)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 10, "Add 10"));
    history->Undo();
    EXPECT_EQ(counter, 0);

    bool result = history->Redo();

    EXPECT_TRUE(result);
    EXPECT_EQ(counter, 10);
    EXPECT_EQ(history->GetCurrentIndex(), 1);
    EXPECT_TRUE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, RedoMultipleCommands)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 2, "Add 2"));

    history->Undo();
    history->Undo();
    history->Undo();
    EXPECT_EQ(counter, 0);

    history->Redo();
    EXPECT_EQ(counter, 5);

    history->Redo();
    EXPECT_EQ(counter, 8);

    history->Redo();
    EXPECT_EQ(counter, 10);

    EXPECT_FALSE(history->CanRedo());
    EXPECT_TRUE(history->CanUndo());
}

TEST_F(CommandHistoryTest, RedoWhenNoRedoAvailable)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 10, "Add 10"));

    bool result = history->Redo();

    EXPECT_FALSE(result);
    EXPECT_EQ(counter, 10);
}

// ============================================================================
// Linear History Model Tests (New Command Clears Redo)
// ============================================================================

TEST_F(CommandHistoryTest, NewCommandClearsRedoStack)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    EXPECT_EQ(counter, 8);

    history->Undo(); // Counter = 5
    EXPECT_EQ(counter, 5);
    EXPECT_TRUE(history->CanRedo());

    // Execute new command - should clear redo stack
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 10, "Add 10"));
    EXPECT_EQ(counter, 15); // 5 + 10

    EXPECT_FALSE(history->CanRedo()); // Redo stack should be cleared
    EXPECT_EQ(history->GetHistorySize(), 2);
    EXPECT_EQ(history->GetCurrentIndex(), 2);
}

TEST_F(CommandHistoryTest, NewCommandAfterMultipleUndos)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 1, "Add 1"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 2, "Add 2"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 4, "Add 4"));
    EXPECT_EQ(counter, 10);

    history->Undo();
    history->Undo();
    history->Undo();
    EXPECT_EQ(counter, 1);

    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 100, "Add 100"));
    EXPECT_EQ(counter, 101);

    EXPECT_FALSE(history->CanRedo());
    EXPECT_EQ(history->GetHistorySize(), 2); // Only "Add 1" and "Add 100"
}

// ============================================================================
// History Size Limit Tests
// ============================================================================

TEST_F(CommandHistoryTest, HistorySizeLimitEnforced)
{
    auto limitedHistory = std::make_unique<CommandHistory>(5);

    for (int i = 0; i < 10; ++i)
    {
        limitedHistory->ExecuteCommand(std::make_unique<MockCommand>(counter, 1, "Add 1"));
    }

    EXPECT_EQ(counter, 10);
    EXPECT_EQ(limitedHistory->GetHistorySize(), 5); // Limited to 5
    EXPECT_EQ(limitedHistory->GetCurrentIndex(), 5);
}

TEST_F(CommandHistoryTest, OldestCommandsRemovedWhenLimitExceeded)
{
    auto limitedHistory = std::make_unique<CommandHistory>(3);
    std::string text = "Initial";

    limitedHistory->ExecuteCommand(std::make_unique<StringCommand>(text, "First", "First"));
    limitedHistory->ExecuteCommand(std::make_unique<StringCommand>(text, "Second", "Second"));
    limitedHistory->ExecuteCommand(std::make_unique<StringCommand>(text, "Third", "Third"));
    EXPECT_EQ(text, "Third");

    // This should remove "First" from history
    limitedHistory->ExecuteCommand(std::make_unique<StringCommand>(text, "Fourth", "Fourth"));
    EXPECT_EQ(text, "Fourth");
    EXPECT_EQ(limitedHistory->GetHistorySize(), 3);

    // Undo all - should go back to "First" (the oldest command that remains)
    limitedHistory->Undo(); // Fourth -> Third
    EXPECT_EQ(text, "Third");
    limitedHistory->Undo(); // Third -> Second
    EXPECT_EQ(text, "Second");
    limitedHistory->Undo(); // Second -> First (state before Second was executed)
    EXPECT_EQ(text, "First");

    EXPECT_FALSE(limitedHistory->CanUndo()); // "First" command was removed, can't go further
}

// ============================================================================
// Clear Functionality Tests
// ============================================================================

TEST_F(CommandHistoryTest, ClearEmptyHistory)
{
    history->Clear();

    EXPECT_EQ(history->GetHistorySize(), 0);
    EXPECT_EQ(history->GetCurrentIndex(), 0);
    EXPECT_FALSE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, ClearNonEmptyHistory)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Add 3"));
    EXPECT_EQ(counter, 8);

    history->Clear();

    EXPECT_EQ(counter, 8); // Counter not affected
    EXPECT_EQ(history->GetHistorySize(), 0);
    EXPECT_EQ(history->GetCurrentIndex(), 0);
    EXPECT_FALSE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, ClearAfterUndoRedo)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));
    history->Undo();
    EXPECT_TRUE(history->CanRedo());

    history->Clear();

    EXPECT_FALSE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
    EXPECT_EQ(history->GetHistorySize(), 0);
}

// ============================================================================
// Description Tests
// ============================================================================

TEST_F(CommandHistoryTest, GetUndoDescription)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Create Node"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Move Node"));

    EXPECT_EQ(history->GetUndoDescription(), "Move Node");

    history->Undo();
    EXPECT_EQ(history->GetUndoDescription(), "Create Node");

    history->Undo();
    EXPECT_EQ(history->GetUndoDescription(), "");
}

TEST_F(CommandHistoryTest, GetRedoDescription)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Create Node"));
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 3, "Delete Connection"));

    EXPECT_EQ(history->GetRedoDescription(), ""); // No redo available

    history->Undo();
    EXPECT_EQ(history->GetRedoDescription(), "Delete Connection");

    history->Undo();
    EXPECT_EQ(history->GetRedoDescription(), "Create Node");
}

TEST_F(CommandHistoryTest, DescriptionAfterClear)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Test Command"));
    history->Clear();

    EXPECT_EQ(history->GetUndoDescription(), "");
    EXPECT_EQ(history->GetRedoDescription(), "");
}

// ============================================================================
// Edge Cases and Complex Scenarios
// ============================================================================

TEST_F(CommandHistoryTest, AlternatingUndoRedo)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 10, "Add 10"));

    for (int i = 0; i < 5; ++i)
    {
        history->Undo();
        EXPECT_EQ(counter, 0);
        history->Redo();
        EXPECT_EQ(counter, 10);
    }

    EXPECT_TRUE(history->CanUndo());
    EXPECT_FALSE(history->CanRedo());
}

TEST_F(CommandHistoryTest, ComplexUndoRedoSequence)
{
    std::string text;

    history->ExecuteCommand(std::make_unique<StringCommand>(text, "A", "Set A"));
    history->ExecuteCommand(std::make_unique<StringCommand>(text, "AB", "Set AB"));
    history->ExecuteCommand(std::make_unique<StringCommand>(text, "ABC", "Set ABC"));
    EXPECT_EQ(text, "ABC");

    history->Undo();
    EXPECT_EQ(text, "AB");

    history->Undo();
    EXPECT_EQ(text, "A");

    history->Redo();
    EXPECT_EQ(text, "AB");

    history->ExecuteCommand(std::make_unique<StringCommand>(text, "ABX", "Set ABX"));
    EXPECT_EQ(text, "ABX");

    EXPECT_FALSE(history->CanRedo()); // "Set ABC" was cleared
    EXPECT_EQ(history->GetHistorySize(), 3);
}

TEST_F(CommandHistoryTest, StateConsistencyAfterManyOperations)
{
    for (int i = 0; i < 50; ++i)
    {
        history->ExecuteCommand(std::make_unique<MockCommand>(counter, 1, "Increment"));
    }
    EXPECT_EQ(counter, 50);

    for (int i = 0; i < 50; ++i)
    {
        history->Undo();
    }
    EXPECT_EQ(counter, 0);

    for (int i = 0; i < 50; ++i)
    {
        history->Redo();
    }
    EXPECT_EQ(counter, 50);

    EXPECT_FALSE(history->CanRedo());
    EXPECT_TRUE(history->CanUndo());
}

TEST_F(CommandHistoryTest, HistoryNavigationBoundaries)
{
    history->ExecuteCommand(std::make_unique<MockCommand>(counter, 5, "Add 5"));

    // Try undo multiple times at boundary
    history->Undo();
    EXPECT_FALSE(history->Undo());
    EXPECT_FALSE(history->Undo());
    EXPECT_EQ(counter, 0);

    // Try redo multiple times at boundary
    history->Redo();
    EXPECT_FALSE(history->Redo());
    EXPECT_FALSE(history->Redo());
    EXPECT_EQ(counter, 5);
}
