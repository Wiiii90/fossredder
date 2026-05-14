/**
 * @file infra/analysis-image-renderer/src/OpenCvAnalysisImageRendererAdapter.cpp
 * @brief Implements OpenCV analysis image rendering.
 */

#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"

#include "core/constants/analysis.h"
#include "core/constants/export.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace {
// Reuse the existing rendering logic as-is.
double parseNumber(const std::string& text)
{
    try {
        return std::stod(text);
    } catch (...) {
        return 0.0;
    }
}

bool tryParseJsonObject(const std::string& text, nlohmann::json& out)
{
    try {
        out = nlohmann::json::parse(text);
        return out.is_object();
    } catch (...) {
        return false;
    }
}

std::vector<std::vector<std::string>> normalizedRowsForImage(const core::domain::AnalysisResult& result)
{
    using core::constants::analysis::resultFields::kAmountAdjusted;
    using core::constants::analysis::resultFields::kAmountOriginal;
    using core::constants::analysis::resultFields::kMonth;
    using core::constants::analysis::resultFields::kTaxFactor;
    using core::constants::analysis::resultFields::kTaxPercent;
    using core::constants::analysis::resultFields::kTotal;
    using core::constants::analysis::resultFields::kTransactionId;

    std::vector<std::vector<std::string>> rows;
    if (result.type == core::constants::analysis::plotTypes::kPie) {
        rows.push_back({"Category", "Value"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;
            if (row.size() == 1) rows.push_back({row[0], "0"});
            else rows.push_back({row[0], row[1]});
        }
        return rows;
    }

    if (result.type == core::constants::analysis::plotTypes::kHistogram) {
        rows.push_back({"Month", "Total"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;
            std::string month = row[0];
            std::string totalValue = "0";
            if (row.size() > 1) {
                nlohmann::json summary;
                if (tryParseJsonObject(row[1], summary)) {
                    if (summary.contains(kMonth) && summary[kMonth].is_string()) {
                        month = summary[kMonth].get<std::string>();
                    }
                    if (summary.contains(kTotal) && summary[kTotal].is_number()) {
                        totalValue = std::to_string(summary[kTotal].get<double>());
                    }
                } else {
                    totalValue = row[1];
                }
            }
            rows.push_back({month, totalValue});
        }
        return rows;
    }

    if (result.type == core::constants::analysis::kTypeCalculation) {
        rows.push_back({"Label", "OriginalAmount", "AdjustedAmount", "TaxPercent", "TaxFactor", "TransactionId"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;

            std::string label = row[0];
            std::string original = "";
            std::string adjusted = "";
            std::string taxPercent = "";
            std::string taxFactor = "";
            std::string txId = "";

            if (row.size() > 1) {
                nlohmann::json summary;
                if (tryParseJsonObject(row[1], summary)) {
                    if (summary.contains(kAmountOriginal) && summary[kAmountOriginal].is_number()) {
                        original = std::to_string(summary[kAmountOriginal].get<double>());
                    }
                    if (summary.contains(kAmountAdjusted) && summary[kAmountAdjusted].is_number()) {
                        adjusted = std::to_string(summary[kAmountAdjusted].get<double>());
                    }
                    if (summary.contains(kTaxPercent) && summary[kTaxPercent].is_number()) {
                        taxPercent = std::to_string(summary[kTaxPercent].get<double>());
                    }
                    if (summary.contains(kTaxFactor) && summary[kTaxFactor].is_number()) {
                        taxFactor = std::to_string(summary[kTaxFactor].get<double>());
                    }
                    if (summary.contains(kTransactionId) && summary[kTransactionId].is_string()) {
                        txId = summary[kTransactionId].get<std::string>();
                    }
                }
            }

            rows.push_back({label, original, adjusted, taxPercent, taxFactor, txId});
        }
        return rows;
    }

    for (const auto& row : result.table) {
        rows.push_back(row);
    }

    return rows;
}

bool drawTableImage(cv::Mat& image, const std::vector<std::vector<std::string>>& rows)
{
    if (rows.empty()) return false;

    int y = 150;
    const int lineHeight = 30;
    const int maxRows = 16;
    for (size_t r = 0; r < rows.size() && static_cast<int>(r) < maxRows; ++r) {
        std::ostringstream line;
        for (size_t c = 0; c < rows[r].size(); ++c) {
            if (c > 0) line << " | ";
            line << rows[r][c];
        }
        cv::putText(image,
                    line.str(),
                    cv::Point(50, y),
                    cv::FONT_HERSHEY_SIMPLEX,
                    r == 0 ? 0.68 : 0.55,
                    r == 0 ? cv::Scalar(30, 30, 30, 255) : cv::Scalar(70, 70, 70, 255),
                    r == 0 ? 2 : 1,
                    cv::LINE_AA);
        y += lineHeight;
    }
    return true;
}

bool drawPieChartImage(cv::Mat& image, const core::domain::AnalysisResult& result)
{
    struct Slice {
        std::string label;
        double value = 0.0;
    };

    std::vector<Slice> slices;
    for (const auto& row : result.table) {
        if (row.empty()) continue;
        const std::string label = row[0];
        const double value = row.size() > 1 ? std::fabs(parseNumber(row[1])) : 0.0;
        if (value > 0.0) slices.push_back({label, value});
    }
    if (slices.empty()) return false;

    double total = 0.0;
    for (const auto& slice : slices) total += slice.value;
    if (total <= 0.0) return false;

    const int width = image.cols;
    const int height = image.rows;
    const int legendWidth = std::max(280, width / 3);
    const int pieAreaWidth = width - legendWidth;
    const int radiusPx = std::max(120, std::min(pieAreaWidth / 2 - 20, height / 2 - 20));
    const cv::Point center(pieAreaWidth / 2, height / 2);
    const cv::Size radius(radiusPx, radiusPx);
    const std::vector<cv::Scalar> colors = {
        {66, 133, 244, 255}, {219, 68, 55, 255}, {244, 180, 0, 255}, {15, 157, 88, 255},
        {171, 71, 188, 255}, {0, 172, 193, 255}, {255, 112, 67, 255}, {158, 158, 158, 255}
    };

    double startAngle = 0.0;
    for (size_t i = 0; i < slices.size(); ++i) {
        const double angle = 360.0 * (slices[i].value / total);
        const cv::Scalar color = colors[i % colors.size()];
        cv::ellipse(image, center, radius, 0.0, startAngle, startAngle + angle, color, cv::FILLED, cv::LINE_AA);
        startAngle += angle;
    }

    const int legendX = width - legendWidth + 20;
    int legendY = 40;
    const int maxLegendRows = std::max(1, (height - 40) / 30);
    for (size_t i = 0; i < slices.size() && static_cast<int>(i) < maxLegendRows; ++i) {
        const cv::Scalar color = colors[i % colors.size()];
        cv::rectangle(image, cv::Point(legendX, legendY - 16), cv::Point(legendX + 20, legendY + 4), color, cv::FILLED, cv::LINE_AA);
        std::ostringstream label;
        label << slices[i].label << " (" << std::fixed << std::setprecision(1) << (slices[i].value * 100.0 / total) << "%)";
        cv::putText(image, label.str(), cv::Point(legendX + 30, legendY), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(50, 50, 50, 255), 1, cv::LINE_AA);
        legendY += 30;
    }

    return true;
}

bool drawHistogramImage(cv::Mat& image, const core::domain::AnalysisResult& result)
{
    struct Bucket {
        std::string label;
        double value = 0.0;
    };

    std::vector<Bucket> buckets;
    for (const auto& row : result.table) {
        if (row.empty()) continue;

        std::string month = row[0];
        double total = 0.0;
        if (row.size() > 1) {
            nlohmann::json summary;
            if (tryParseJsonObject(row[1], summary)) {
                if (summary.contains(core::constants::analysis::resultFields::kMonth)
                    && summary[core::constants::analysis::resultFields::kMonth].is_string()) {
                    month = summary[core::constants::analysis::resultFields::kMonth].get<std::string>();
                }
                if (summary.contains(core::constants::analysis::resultFields::kTotal)
                    && summary[core::constants::analysis::resultFields::kTotal].is_number()) {
                    total = std::fabs(summary[core::constants::analysis::resultFields::kTotal].get<double>());
                }
            } else {
                total = std::fabs(parseNumber(row[1]));
            }
        }

        buckets.push_back({month, total});
    }
    if (buckets.empty()) return false;

    const int width = image.cols;
    const int height = image.rows;
    const int legendWidth = std::max(260, width / 3);
    const int left = 40;
    const int right = width - legendWidth - 20;
    const int top = 20;
    const int bottom = height - 40;
    cv::line(image, cv::Point(left, bottom), cv::Point(right, bottom), cv::Scalar(110, 110, 110, 255), 2, cv::LINE_AA);
    cv::line(image, cv::Point(left, top), cv::Point(left, bottom), cv::Scalar(110, 110, 110, 255), 2, cv::LINE_AA);

    double maxValue = 0.0;
    for (const auto& bucket : buckets) maxValue = std::max(maxValue, bucket.value);
    if (maxValue <= 0.0) maxValue = 1.0;

    const int count = static_cast<int>(buckets.size());
    const int slotWidth = std::max(16, (right - left - 20) / std::max(1, count));
    const int barWidth = std::max(10, slotWidth - 12);
    const cv::Scalar barColor(66, 133, 244, 255);
    for (int i = 0; i < count; ++i) {
        const auto& bucket = buckets[static_cast<size_t>(i)];
        const int barHeight = static_cast<int>(((bottom - top - 10) * bucket.value) / maxValue);
        const int x1 = left + 10 + (i * slotWidth);
        const int x2 = x1 + barWidth;
        const int y1 = bottom - barHeight;

        cv::rectangle(image, cv::Point(x1, y1), cv::Point(x2, bottom - 1), barColor, cv::FILLED, cv::LINE_AA);

        if (i % std::max(1, count / 12) == 0) {
            cv::putText(image, bucket.label, cv::Point(x1, bottom + 16), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(70, 70, 70, 255), 1, cv::LINE_AA);
        }
    }

    const int legendX = width - legendWidth + 20;
    cv::rectangle(image, cv::Point(legendX, 26), cv::Point(legendX + 20, 46), barColor, cv::FILLED, cv::LINE_AA);
    cv::putText(image, "Total", cv::Point(legendX + 30, 42), cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(50, 50, 50, 255), 1, cv::LINE_AA);

    int legendY = 80;
    const int maxLegendRows = std::max(1, (height - 80) / 24);
    for (int i = 0; i < count && i < maxLegendRows; ++i) {
        const auto& bucket = buckets[static_cast<size_t>(i)];
        std::ostringstream label;
        label << bucket.label << ": " << std::fixed << std::setprecision(2) << bucket.value;
        cv::putText(image, label.str(), cv::Point(legendX, legendY), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(60, 60, 60, 255), 1, cv::LINE_AA);
        legendY += 24;
    }

    return true;
}

bool writeImageFromResult(const std::filesystem::path& outputPath,
                          const core::domain::AnalysisResult& result)
{
    constexpr int width = 1280;
    constexpr int height = 720;
    const bool asPng = outputPath.has_extension()
        && outputPath.extension().string() == ".png";

    cv::Mat image;
    if (asPng) image = cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    else image = cv::Mat(height, width, CV_8UC3, cv::Scalar(250, 250, 250));

    bool ok = false;
    if (result.type == core::constants::analysis::plotTypes::kPie) {
        ok = drawPieChartImage(image, result);
    } else if (result.type == core::constants::analysis::plotTypes::kHistogram) {
        ok = drawHistogramImage(image, result);
    } else {
        ok = drawTableImage(image, normalizedRowsForImage(result));
    }

    if (!ok) return false;

    return cv::imwrite(outputPath.string(), image);
}

} // namespace

namespace infra::analysis_image_renderer {

bool OpenCvAnalysisImageRendererAdapter::writeAnalysisImage(const std::filesystem::path& outputPath,
                                                            const std::string& title,
                                                            const core::domain::AnalysisResult& result) const
{
    (void)title;
    return writeImageFromResult(outputPath, result);
}

} // namespace infra::analysis_image_renderer
