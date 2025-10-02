#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>

namespace VisionCraft::Engine
{
    /**
     * @brief Node for converting images to grayscale.
     *
     * GrayscaleNode converts color images to grayscale using OpenCV's color
     * conversion functions. It automatically handles different input formats
     * and provides options for different conversion methods.
     */
    class GrayscaleNode : public Node
    {
    public:
        /**
         * @brief Constructs a GrayscaleNode with the given ID and name.
         * @param id Unique identifier for this node
         * @param name Display name for this node
         */
        GrayscaleNode(NodeId id, const std::string &name = "Grayscale");

        /**
         * @brief Virtual destructor.
         */
        virtual ~GrayscaleNode() = default;

        /**
         * @brief Processes the input image by converting it to grayscale.
         *
         * Reads input from the "Input" slot, converts to grayscale,
         * and writes the result to the "Output" slot.
         * If the input is already grayscale, it passes through unchanged.
         */
        void Process() override;

    private:
        /**
         * @brief Converts conversion method string to OpenCV constant.
         * @param methodStr String representation of conversion method
         * @return OpenCV color conversion constant
         */
        int GetConversionMethod(const std::string &methodStr) const;
    };
} // namespace VisionCraft::Engine