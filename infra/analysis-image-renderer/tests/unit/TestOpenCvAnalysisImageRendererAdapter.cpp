/**
 * @file infra/analysis-image-renderer/tests/unit/TestOpenCvAnalysisImageRendererAdapter.cpp
 * @brief Tests for the OpenCV analysis image renderer adapter.
 */

#include <gtest/gtest.h>

#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"
#include "core/constants/analysis.h"

#include <opencv2/imgcodecs.hpp>

namespace infra::analysis_image_renderer::tests {
namespace {

class OpenCvAnalysisImageRendererAdapterTest : public ::testing::TestWithParam<std::string> {
protected:
    static std::filesystem::path makeTempDir(const std::string& stem) {
        auto dir = std::filesystem::temp_directory_path() / stem;
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
        return dir;
    }

    static core::domain::AnalysisResult makeResult(const std::string& type) {
        core::domain::AnalysisResult result;
        result.type = type;
        if (type == core::constants::analysis::plotTypes::kPie) {
            result.table = {
                {"Rent", "2.0"},
                {"Food", "1.0"},
            };
        } else if (type == core::constants::analysis::plotTypes::kHistogram) {
            result.table = {
                {"January", R"({"month":"January","total":120.5})"},
                {"February", R"({"month":"February","total":98.0})"},
            };
        } else {
            result.type = core::constants::analysis::kTypeCalculation;
            result.table = {
                {"Adjustment", R"({"amountOriginal":100.0,"amountAdjusted":80.0,"taxPercent":19.0,"taxFactor":0.19,"transactionId":"tx-1"})"},
                {"Refund", R"({"amountOriginal":25.0,"amountAdjusted":25.0,"taxPercent":0.0,"taxFactor":0.0,"transactionId":"tx-2"})"},
            };
        }
        return result;
    }
};

TEST_P(OpenCvAnalysisImageRendererAdapterTest, WritesRenderableImageForSupportedAnalysisTypes) {
    const auto tempDir = makeTempDir("fossredder_analysis_image_renderer_test");
    const auto outputPath = tempDir / (GetParam() + ".png");
    const auto result = makeResult(GetParam());

    OpenCvAnalysisImageRendererAdapter adapter;
    ASSERT_TRUE(adapter.writeAnalysisImage(outputPath, "Analysis", result));
    ASSERT_TRUE(std::filesystem::exists(outputPath));
    ASSERT_GT(std::filesystem::file_size(outputPath), 0u);

    const auto image = cv::imread(outputPath.string(), cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(image.empty());
}

INSTANTIATE_TEST_SUITE_P(
    SupportedTypes,
    OpenCvAnalysisImageRendererAdapterTest,
    ::testing::Values(
        std::string(core::constants::analysis::plotTypes::kPie),
        std::string(core::constants::analysis::plotTypes::kHistogram),
        std::string(core::constants::analysis::kTypeCalculation)));

} // namespace
} // namespace infra::analysis_image_renderer::tests
