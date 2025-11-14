#pragma once

#include "UI/Widgets/NodeEditorTypes.h"
#include "Command.h"

#include <functional>
#include <string>

namespace VisionCraft::Editor::Commands
{
    /**
     * @brief Command for creating a connection between nodes.
     */
    class CreateConnectionCommand : public Command
    {
    public:
        /** @brief Function that creates a connection */
        using ConnectionCreator = std::function<void(const UI::Widgets::PinId &, const UI::Widgets::PinId &)>;
        /** @brief Function that removes a connection */
        using ConnectionRemover = std::function<void(const UI::Widgets::NodeConnection &)>;

        CreateConnectionCommand(const UI::Widgets::PinId &outputPin,
            const UI::Widgets::PinId &inputPin,
            ConnectionCreator creator,
            ConnectionRemover remover);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        UI::Widgets::PinId outputPin;                  ///< Source pin
        UI::Widgets::PinId inputPin;                   ///< Target pin
        ConnectionCreator creator;                     ///< Creates connection
        ConnectionRemover remover;                     ///< Removes connection
        UI::Widgets::NodeConnection createdConnection; ///< Stored for undo
    };

    /**
     * @brief Command for deleting a connection.
     */
    class DeleteConnectionCommand : public Command
    {
    public:
        /** @brief Function that creates a connection */
        using ConnectionCreator = std::function<void(const UI::Widgets::PinId &, const UI::Widgets::PinId &)>;
        /** @brief Function that removes a connection */
        using ConnectionRemover = std::function<void(const UI::Widgets::NodeConnection &)>;

        DeleteConnectionCommand(const UI::Widgets::NodeConnection &connection,
            ConnectionCreator creator,
            ConnectionRemover remover);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        UI::Widgets::NodeConnection connection; ///< Connection to delete
        ConnectionCreator creator;              ///< Creates connection
        ConnectionRemover remover;              ///< Removes connection
    };

} // namespace VisionCraft::Editor::Commands
