#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Morphological operation types.
     */
    enum class MorphOperation : int
    {
        Erode = 0,
        Dilate = 1,
        Open = 2,
        Close = 3,
        Gradient = 4,
        TopHat = 5,
        BlackHat = 6
    };

    /**
     * @brief Node for Morphological operations (Erode, Dilate, Open, Close, etc.).
     */
    class MorphologyNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs Morphology node.
         * @param id Node ID
         * @param name Node name
         */
        MorphologyNode(Nodes::NodeId id, const std::string &name = "Morphology");

        /**
         * @brief Virtual destructor.
         */
        virtual ~MorphologyNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "Morphology";
        }

        /**
         * @brief Processes input image using Morphological operations.
         */
        void Process() override;
    };
} // namespace VisionCraft::Vision::Algorithms
