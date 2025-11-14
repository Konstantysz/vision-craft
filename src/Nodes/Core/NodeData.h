#pragma once

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace VisionCraft::Nodes
{
    /**
     * @brief Type-safe variant representing all data types that can flow between nodes.
     *
     * This variant uses std::variant (C++17/20) for zero-cost type erasure without heap allocation.
     * Adding new types is easy - just add to this variant and recompile.
     *
     * Current supported types:
     * - cv::Mat: Images (the primary data type for vision processing)
     * - double: Double precision scalar values (thresholds, parameters, etc.)
     * - float: Single precision scalar values
     * - int: Integer values (counts, indices, etc.)
     * - bool: Boolean values (flags, toggles, etc.)
     * - std::string: Text data (filenames, labels, etc.)
     * - std::filesystem::path: File paths
     * - std::vector<cv::Point>: Future support for contours, keypoints, etc.
     *
     * @note std::variant is stack-allocated and has zero runtime overhead compared to dynamic_cast
     * @note Adding types here requires recompilation but does NOT require modifying NodeEditor
     */
    using NodeData = std::variant<std::monostate, // Empty/uninitialized state
        cv::Mat,                                  // Images
        double,                                   // Double precision scalars
        float,                                    // Single precision scalars
        int,                                      // Integer values
        bool,                                     // Boolean values
        std::string,                              // Text data
        std::filesystem::path,                    // File paths
        std::vector<cv::Point>                    // Contours, keypoints (future)
        >;

} // namespace VisionCraft::Nodes
