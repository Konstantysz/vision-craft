#pragma once

#include "Editor/NodeEditorTypes.h"
#include "Command.h"

#include <functional>
#include <string>

namespace VisionCraft
{
    /**
     * @brief Command for creating a connection between nodes.
     */
    class CreateConnectionCommand : public Command
    {
    public:
        /** @brief Function that creates a connection */
        using ConnectionCreator = std::function<void(const PinId &, const PinId &)>;
        /** @brief Function that removes a connection */
        using ConnectionRemover = std::function<void(const NodeConnection &)>;

        CreateConnectionCommand(const PinId &outputPin,
            const PinId &inputPin,
            ConnectionCreator creator,
            ConnectionRemover remover);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        PinId outputPin;                  ///< Source pin
        PinId inputPin;                   ///< Target pin
        ConnectionCreator creator;        ///< Creates connection
        ConnectionRemover remover;        ///< Removes connection
        NodeConnection createdConnection; ///< Stored for undo
    };

    /**
     * @brief Command for deleting a connection.
     */
    class DeleteConnectionCommand : public Command
    {
    public:
        /** @brief Function that creates a connection */
        using ConnectionCreator = std::function<void(const PinId &, const PinId &)>;
        /** @brief Function that removes a connection */
        using ConnectionRemover = std::function<void(const NodeConnection &)>;

        DeleteConnectionCommand(const NodeConnection &connection, ConnectionCreator creator, ConnectionRemover remover);

        void Execute() override;

        void Undo() override;

        [[nodiscard]] std::string GetDescription() const override;

    private:
        NodeConnection connection; ///< Connection to delete
        ConnectionCreator creator; ///< Creates connection
        ConnectionRemover remover; ///< Removes connection
    };

} // namespace VisionCraft
