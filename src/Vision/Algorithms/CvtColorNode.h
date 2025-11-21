#pragma once

#include "Nodes/Core/Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Vision::Algorithms
{
    /**
     * @brief Color conversion types.
     */
    enum class ColorConversion : int
    {
        BGR2GRAY = 0,
        GRAY2BGR = 1,
        BGR2RGB = 2,
        RGB2BGR = 3,
        BGR2HSV = 4,
        HSV2BGR = 5
    };

    /**
     * @brief Conversion information including required channels.
     */
    struct ConversionInfo
    {
        cv::ColorConversionCodes code;
        int requiredChannels;
        std::string_view name;
    };

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
            return "CvtColor";
        }

        /**
         * @brief Processes input image using Color Conversion.
         */
        void Process() override;
    };
} // namespace VisionCraft::Vision::Algorithms
