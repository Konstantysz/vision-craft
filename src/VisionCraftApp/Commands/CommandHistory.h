#pragma once

#include "Command.h"

#include <memory>
#include <vector>

namespace VisionCraft
{
    /**
     * @brief Manages command history for undo/redo functionality.
     *
     * Maintains a stack of executed commands and supports undo/redo operations.
     * Uses a linear history model - executing a new command after undo
     * clears the redo stack.
     */
    class CommandHistory
    {
    public:
        /**
         * @brief Default constructor.
         * @param maxHistorySize Maximum number of commands to keep (default: 100)
         */
        explicit CommandHistory(size_t maxHistorySize = 100);

        /**
         * @brief Executes a command and adds it to history.
         * @param command Command to execute (ownership transferred)
         *
         * Executes the command immediately, then stores it in history.
         * Clears any redo history if we're not at the end of the stack.
         */
        void ExecuteCommand(std::unique_ptr<Command> command);

        /**
         * @brief Undoes the most recent command.
         * @return True if undo was performed, false if nothing to undo
         */
        bool Undo();

        /**
         * @brief Redoes the most recently undone command.
         * @return True if redo was performed, false if nothing to redo
         */
        bool Redo();

        /**
         * @brief Checks if undo is available.
         * @return True if there are commands to undo
         */
        [[nodiscard]] bool CanUndo() const;

        /**
         * @brief Checks if redo is available.
         * @return True if there are commands to redo
         */
        [[nodiscard]] bool CanRedo() const;

        /**
         * @brief Gets description of command that would be undone.
         * @return Description string, or empty if nothing to undo
         */
        [[nodiscard]] std::string GetUndoDescription() const;

        /**
         * @brief Gets description of command that would be redone.
         * @return Description string, or empty if nothing to redo
         */
        [[nodiscard]] std::string GetRedoDescription() const;

        /**
         * @brief Clears all command history.
         *
         * Removes all commands from history. Use when loading a new file
         * or when you want to reset the undo stack.
         */
        void Clear();

        /**
         * @brief Gets current history size.
         * @return Number of commands in history
         */
        [[nodiscard]] size_t GetHistorySize() const
        {
            return history.size();
        }

        /**
         * @brief Gets current position in history.
         * @return Current index (0 = no commands executed)
         */
        [[nodiscard]] size_t GetCurrentIndex() const
        {
            return currentIndex;
        }

    private:
        std::vector<std::unique_ptr<Command>> history; ///< Command history stack
        size_t currentIndex = 0;                       ///< Current position in history (0 = nothing executed)
        size_t maxHistorySize;                         ///< Maximum number of commands to keep

        /**
         * @brief Removes oldest commands if history exceeds max size.
         */
        void TrimHistory();
    };

} // namespace VisionCraft