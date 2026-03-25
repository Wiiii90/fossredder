/**
 * @file services/opencv/tests/unit/TestDetectEngine.cpp
 * @brief Regression coverage for OpenCV-based detection input normalization.
 */

#include "gtest/gtest.h"

#include <opencv2/opencv.hpp>

#include "opencv/DetectEngine.h"

TEST(DetectEngineTests, DetectTextBlocksAcceptsColorImage)
{
    cv::Mat colorImage(32, 32, CV_8UC3, cv::Scalar(200, 120, 40));

    EXPECT_NO_THROW({
        (void)opencv::DetectEngine::DetectTextBlocks(colorImage);
    });
}

TEST(DetectEngineTests, DetectTablesAcceptsColorImage)
{
    cv::Mat colorImage(48, 48, CV_8UC4, cv::Scalar(10, 20, 30, 255));

    EXPECT_NO_THROW({
        (void)opencv::DetectEngine::DetectTables(colorImage, "synthetic.png");
    });
}
