#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Node for Color Space Conversion.
     */
    class CvtColorNode : public Nodes::Node
    {
    public:
        /**
         * @brief Constructs CvtColor node.
         * @param id Node ID
         * @param name Node name
         */
        CvtColorNode(Nodes::NodeId id, const std::string &name = "Convert Color");

        /**
         * @brief Virtual destructor.
         */
        virtual ~CvtColorNode() = default;

        /**
         * @brief Returns node type identifier.
         * @return Type string
         */
        [[nodiscard]] std::string GetType() const override
        {
            return "CvtColorNode";
        }

        /**
         * @brief Processes input image using Color Conversion.
         */
        void Process() override;

    private:
        cv::Mat inputImage;  ///< Input image
        cv::Mat outputImage; ///< Resulting image
    };
} // namespace VisionCraft::Vision::Algorithms
