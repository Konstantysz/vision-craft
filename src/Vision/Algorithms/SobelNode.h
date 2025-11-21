#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for Sobel edge detection.
     */
    class SobelNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs Sobel edge detection node.
         * @param id Node ID
         * @param name Node name
         */
        SobelNode(Nodes::NodeId id, const std::string &name = "Sobel Operator");

        /**
         * @brief Virtual destructor.
         */
        virtual ~SobelNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "Sobel";
        }

        /**
         * @brief Processes input image using Sobel operator.
         */
        void Process() override;
    };
} // namespace VisionCraft::Vision::Algorithms
