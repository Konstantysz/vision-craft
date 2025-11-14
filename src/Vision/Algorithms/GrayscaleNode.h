#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for converting images to grayscale.
     */
    class GrayscaleNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs grayscale conversion node.
         * @param id Node ID
         * @param name Node name
         */
        GrayscaleNode(Nodes::NodeId id, const std::string &name = "Grayscale");

        /**
         * @brief Virtual destructor.
         */
        virtual ~GrayscaleNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "GrayscaleNode";
        }

        /**
         * @brief Processes input image by converting to grayscale.
         */
        void Process() override;

    private:
        /**
         * @brief Converts method string to OpenCV constant.
         * @param methodStr Conversion method string
         * @return OpenCV color conversion constant
         */
        int GetConversionMethod(const std::string &methodStr) const;
    };
} // namespace VisionCraft::Vision::Algorithms
