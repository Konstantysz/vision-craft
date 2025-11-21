#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Interpolation method types.
     */
    enum class InterpolationMethod : int
    {
        Linear = 0,
        Nearest = 1,
        Cubic = 2,
        Area = 3,
        Lanczos4 = 4
    };

    /**
     * @brief Node for Image Resizing.
     */
    class ResizeNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs Resize node.
         * @param id Node ID
         * @param name Node name
         */
        ResizeNode(Nodes::NodeId id, const std::string &name = "Resize");

        /**
         * @brief Virtual destructor.
         */
        virtual ~ResizeNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "Resize";
        }

        /**
         * @brief Processes input image using Resize.
         */
        void Process() override;
    };
} // namespace VisionCraft::Vision::Algorithms
