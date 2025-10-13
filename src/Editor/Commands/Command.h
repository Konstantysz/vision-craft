#pragma once

#include <memory>
#include <string>

namespace VisionCraft
{
    /**
     * @brief Base interface for all undoable commands.
     *
     * Commands encapsulate operations that modify the editor state.
     * Each command knows how to execute itself and undo its effects.
     */
    class Command
    {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~Command() = default;

        /**
         * @brief Executes the command.
         *
         * This method performs the operation and modifies the editor state.
         * Should be idempotent - calling multiple times has same effect.
         */
        virtual void Execute() = 0;

        /**
         * @brief Undoes the command.
         *
         * Reverses the effects of Execute(), restoring previous state.
         * Must be callable even after Execute() has been called.
         */
        virtual void Undo() = 0;

        /**
         * @brief Returns a human-readable description of this command.
         * @return Description string (e.g., "Create Node", "Delete Connection")
         */
        [[nodiscard]] virtual std::string GetDescription() const = 0;

    protected:
        /**
         * @brief Protected constructor - only derived classes can instantiate.
         */
        Command() = default;

        // Non-copyable, non-movable (commands should be unique)
        Command(const Command &) = delete;
        Command &operator=(const Command &) = delete;
        Command(Command &&) = delete;
        Command &operator=(Command &&) = delete;
    };

} // namespace VisionCraft
