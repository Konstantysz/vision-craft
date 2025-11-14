#include "ConnectionCommands.h"

namespace VisionCraft::Editor::Commands
{
    // CreateConnectionCommand

    CreateConnectionCommand::CreateConnectionCommand(const UI::Widgets::PinId &outputPin,
        const UI::Widgets::PinId &inputPin,
        ConnectionCreator creator,
        ConnectionRemover remover)
        : outputPin(outputPin), inputPin(inputPin), creator(std::move(creator)), remover(std::move(remover))
    {
        createdConnection.outputPin = outputPin;
        createdConnection.inputPin = inputPin;
    }

    void CreateConnectionCommand::Execute()
    {
        creator(outputPin, inputPin);
    }

    void CreateConnectionCommand::Undo()
    {
        remover(createdConnection);
    }

    std::string CreateConnectionCommand::GetDescription() const
    {
        return "Create Connection";
    }

    // DeleteConnectionCommand

    DeleteConnectionCommand::DeleteConnectionCommand(const UI::Widgets::NodeConnection &connection,
        ConnectionCreator creator,
        ConnectionRemover remover)
        : connection(connection), creator(std::move(creator)), remover(std::move(remover))
    {
    }

    void DeleteConnectionCommand::Execute()
    {
        remover(connection);
    }

    void DeleteConnectionCommand::Undo()
    {
        creator(connection.outputPin, connection.inputPin);
    }

    std::string DeleteConnectionCommand::GetDescription() const
    {
        return "Delete Connection";
    }

} // namespace VisionCraft::Editor::Commands
