#include "CommandHistory.h"

namespace VisionCraft
{
    CommandHistory::CommandHistory(size_t maxHistorySize) : maxHistorySize(maxHistorySize)
    {
    }

    void CommandHistory::ExecuteCommand(std::unique_ptr<Command> command)
    {
        if (!command)
        {
            return;
        }

        command->Execute();

        if (currentIndex < history.size())
        {
            history.erase(history.begin() + currentIndex, history.end());
        }

        history.push_back(std::move(command));
        ++currentIndex;

        TrimHistory();
    }

    bool CommandHistory::Undo()
    {
        if (!CanUndo())
        {
            return false;
        }

        --currentIndex;
        history[currentIndex]->Undo();

        return true;
    }

    bool CommandHistory::Redo()
    {
        if (!CanRedo())
        {
            return false;
        }

        history[currentIndex]->Execute();
        ++currentIndex;

        return true;
    }

    bool CommandHistory::CanUndo() const
    {
        return currentIndex > 0;
    }

    bool CommandHistory::CanRedo() const
    {
        return currentIndex < history.size();
    }

    std::string CommandHistory::GetUndoDescription() const
    {
        if (!CanUndo())
        {
            return "";
        }

        return history[currentIndex - 1]->GetDescription();
    }

    std::string CommandHistory::GetRedoDescription() const
    {
        if (!CanRedo())
        {
            return "";
        }

        return history[currentIndex]->GetDescription();
    }

    void CommandHistory::Clear()
    {
        history.clear();
        currentIndex = 0;
    }

    void CommandHistory::TrimHistory()
    {
        if (history.size() <= maxHistorySize)
        {
            return;
        }

        const size_t toRemove = history.size() - maxHistorySize;
        history.erase(history.begin(), history.begin() + toRemove);

        if (currentIndex > toRemove)
        {
            currentIndex -= toRemove;
        }
        else
        {
            currentIndex = 0;
        }
    }

} // namespace VisionCraft