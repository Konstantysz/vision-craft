#pragma once

#include <functional>
#include <optional>
#include <string>

namespace VisionCraft::UI::Widgets
{
    /**
     * @brief Result of file dialog operations.
     */
    struct FileDialogResult
    {
        enum class Action
        {
            None,
            Save,
            Load,
            Cancel
        };

        Action action = Action::None;
        std::string filepath; ///< Selected file path
    };

    /**
     * @brief Manages save/load file dialogs for graph operations.
     *
     * Separates file I/O UI concerns from layer logic.
     * Handles file path input, validation, and dialog state.
     */
    class FileDialogManager
    {
    public:
        /**
         * @brief Renders save dialog and returns result.
         * @return Result indicating action taken and filepath
         */
        [[nodiscard]] FileDialogResult RenderSaveDialog();

        /**
         * @brief Renders load dialog and returns result.
         * @return Result indicating action taken and filepath
         */
        [[nodiscard]] FileDialogResult RenderLoadDialog();

        /**
         * @brief Opens save dialog.
         */
        void OpenSaveDialog();

        /**
         * @brief Opens load dialog.
         */
        void OpenLoadDialog();

        /**
         * @brief Checks if save dialog is open.
         * @return True if save dialog is showing
         */
        [[nodiscard]] bool IsSaveDialogOpen() const;

        /**
         * @brief Checks if load dialog is open.
         * @return True if load dialog is showing
         */
        [[nodiscard]] bool IsLoadDialogOpen() const;

    private:
        bool showSaveDialog = false;
        bool showLoadDialog = false;
        char filePathBuffer[512] = "";

        /**
         * @brief Ensures filepath has .json extension.
         * @param filepath Input filepath
         * @return Filepath with .json extension
         */
        [[nodiscard]] static std::string EnsureJsonExtension(const std::string &filepath);

        /**
         * @brief Clears file path buffer.
         */
        void ClearBuffer();
    };
} // namespace VisionCraft::UI::Widgets
