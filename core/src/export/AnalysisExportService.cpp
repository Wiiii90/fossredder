/**
 * @file core/src/export/AnalysisExportService.cpp
 * @brief Implements analysis export to CSV/XLSX/JPG/PNG plus optional packaging.
 */

#include "core/export/AnalysisExportService.h"

#include "core/application/AnalysisService.h"
#include "core/constants/CoreDefaults.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <cmath>

#include <zip.h>

#include <xlnt/xlnt.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

#include <nlohmann/json.hpp>

#include "core/models/AppState.h"

namespace {

std::string safeFilePart(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        const bool ok = (c >= 'a' && c <= 'z')
                     || (c >= 'A' && c <= 'Z')
                     || (c >= '0' && c <= '9')
                     || c == '_' || c == '-' || c == '.';
        out.push_back(ok ? c : '_');
    }
    if (out.empty()) return "entry";
    return out;
}

std::string extensionFor(core::exporting::AnalysisExportFormat format)
{
    using core::exporting::AnalysisExportFormat;
    switch (format) {
    case AnalysisExportFormat::Csv: return "csv";
    case AnalysisExportFormat::Xlsx: return "xlsx";
    case AnalysisExportFormat::Jpg: return "jpg";
    case AnalysisExportFormat::Png: return "png";
    }
    return "dat";
}

bool writeCsvTable(const std::filesystem::path& outputPath,
                   const std::vector<std::vector<std::string>>& rows)
{
    std::ofstream stream(outputPath, std::ios::binary);
    if (!stream) return false;

    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) stream << ';';
            stream << row[i];
        }
        stream << '\n';
    }

    stream.close();
    return !!stream;
}

bool writePseudoXlsxTable(const std::filesystem::path& outputPath,
                          const std::vector<std::vector<std::string>>& rows)
{
    try {
        xlnt::workbook workbook;
        xlnt::worksheet worksheet = workbook.active_sheet();
        worksheet.title("Analysis");

        for (size_t row = 0; row < rows.size(); ++row) {
            const auto& columns = rows[row];
            for (size_t column = 0; column < columns.size(); ++column) {
                worksheet.cell(static_cast<int>(row + 1), static_cast<int>(column + 1)).value(columns[column]);
            }
        }

        workbook.save(outputPath.string());
        return true;
    } catch (...) {
        return false;
    }
}

std::string joinedMetrics(const std::map<std::string, double>& metrics)
{
    std::ostringstream stream;
    bool first = true;
    for (const auto& [key, value] : metrics) {
        if (!first) stream << " | ";
        first = false;
        stream << key << ": " << std::fixed << std::setprecision(2) << value;
    }
    return stream.str();
}

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

std::vector<std::vector<std::string>> normalizedRowsForExport(const core::domain::AnalysisResult& result)
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

    if (result.type == core::constants::analysis::kTypeCalc) {
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

std::unordered_map<std::string, std::string> propertyNameById(const core::domain::AppState& state)
{
    std::unordered_map<std::string, std::string> out;
    out.reserve(state.properties.size());
    for (const auto& property : state.properties) {
        if (!property || property->id.empty()) continue;
        out[property->id] = property->name.empty() ? property->id : property->name;
    }
    return out;
}

std::vector<std::vector<std::string>> normalizedRowsForExport(const core::domain::AnalysisResult& result,
                                                              const core::domain::AppState& state)
{
    if (result.type != core::constants::analysis::kTypeTab) {
        return normalizedRowsForExport(result);
    }

    std::set<std::string> contractTypes;
    std::map<std::string, std::map<std::string, double>> amountsByProperty;
    const auto nameById = propertyNameById(state);

    for (const auto& tx : result.transactions) {
        const std::string contractType = !tx.contractType.empty()
            ? tx.contractType
            : std::string(core::constants::exportFlow::labels::kUnassigned);
        contractTypes.insert(contractType);

        for (const auto& propertyId : tx.propertyIds) {
            if (propertyId.empty()) continue;
            const auto it = nameById.find(propertyId);
            const std::string propertyName = it != nameById.end() ? it->second : propertyId;
            amountsByProperty[propertyName][contractType] += tx.amount;
        }
    }

    std::vector<std::vector<std::string>> rows;
    rows.push_back({std::string(core::constants::exportFlow::labels::kPropertyHeader), "contract.type", std::string(core::constants::exportFlow::labels::kTotal)});

    for (const auto& [propertyName, byContract] : amountsByProperty) {
        double total = 0.0;
        for (const auto& contractType : contractTypes) {
            const auto it = byContract.find(contractType);
            total += (it != byContract.end()) ? it->second : 0.0;
        }

        rows.push_back({propertyName, std::string(core::constants::exportFlow::labels::kTotal), std::to_string(total)});
        for (const auto& contractType : contractTypes) {
            const auto it = byContract.find(contractType);
            const double value = it != byContract.end() ? it->second : 0.0;
            rows.push_back({propertyName, contractType, std::to_string(value)});
        }
    }

    return rows;
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

bool writeImageFromMetrics(const std::filesystem::path& outputPath,
                           const std::string& title,
                           const std::map<std::string, double>& metrics)
{
    constexpr int width = 1280;
    constexpr int height = 720;
    cv::Mat image(height, width, CV_8UC3, cv::Scalar(250, 250, 250));

    cv::putText(image,
                title,
                cv::Point(40, 80),
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(40, 40, 40),
                2,
                cv::LINE_AA);

    const std::string metricsLine = joinedMetrics(metrics);
    cv::putText(image,
                metricsLine,
                cv::Point(40, 140),
                cv::FONT_HERSHEY_SIMPLEX,
                0.75,
                cv::Scalar(60, 60, 60),
                2,
                cv::LINE_AA);

    int y = 210;
    int index = 1;
    for (const auto& [key, value] : metrics) {
        std::ostringstream row;
        row << index++ << ". " << key << " = " << std::fixed << std::setprecision(2) << value;
        cv::putText(image,
                    row.str(),
                    cv::Point(60, y),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.65,
                    cv::Scalar(90, 90, 90),
                    2,
                    cv::LINE_AA);
        y += 38;
        if (y > height - 40) break;
    }

    return cv::imwrite(outputPath.string(), image);
}

bool writeImageFromResult(const std::filesystem::path& outputPath,
                          const std::string& title,
                          const core::domain::AnalysisResult& result)
{
    (void)title;
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
        ok = drawTableImage(image, normalizedRowsForExport(result));
    }

    if (!ok) return false;

    return cv::imwrite(outputPath.string(), image);
}

bool createZipArchive(const std::filesystem::path& sourceDir,
                      const std::filesystem::path& outputArchive)
{
    int errorCode = 0;
    zip_t* archive = zip_open(outputArchive.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorCode);
    if (!archive) return false;

    bool success = true;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file()) continue;

        const std::filesystem::path absolutePath = entry.path();
        std::filesystem::path relativePath;
        try {
            relativePath = std::filesystem::relative(absolutePath, sourceDir);
        } catch (...) {
            relativePath = absolutePath.filename();
        }

        zip_source_t* source = zip_source_file(archive, absolutePath.string().c_str(), 0, 0);
        if (!source) {
            success = false;
            break;
        }

        const std::string zipPath = relativePath.generic_string();
        if (zip_file_add(archive, zipPath.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
            zip_source_free(source);
            success = false;
            break;
        }
    }

    if (!success) {
        zip_discard(archive);
        return false;
    }

    return zip_close(archive) == 0 && std::filesystem::exists(outputArchive);
}

std::unordered_map<std::string, std::string> annualFolderNames(const core::domain::AppState& state)
{
    std::unordered_map<std::string, std::string> folderByAnnualId;
    std::set<std::string> usedFolderNames;

    for (const auto& annual : state.annuals) {
        if (!annual || annual->id.empty()) continue;

        const std::string baseName = safeFilePart(!annual->name.empty() ? annual->name : annual->id);
        std::string candidate = baseName;
        if (usedFolderNames.find(candidate) != usedFolderNames.end()) {
            std::string suffix = annual->id;
            if (suffix.size() > 8) suffix = suffix.substr(0, 8);
            candidate = baseName + "_" + safeFilePart(suffix);
        }

        int idx = 2;
        while (usedFolderNames.find(candidate) != usedFolderNames.end()) {
            candidate = baseName + "_" + std::to_string(idx++);
        }

        usedFolderNames.insert(candidate);
        folderByAnnualId[annual->id] = candidate;
    }

    return folderByAnnualId;
}

std::filesystem::path outputPathForItem(const std::filesystem::path& baseOutput,
                                        const std::unordered_map<std::string, std::string>& folderByAnnualId,
                                        const core::exporting::AnalysisExportItem& item)
{
    const std::string fileStem = safeFilePart(!item.name.empty() ? item.name : item.analysisId);
    const std::string ext = extensionFor(item.format);

    if (item.annualId.empty()) {
        return baseOutput / (fileStem + "." + ext);
    }

    const auto it = folderByAnnualId.find(item.annualId);
    const std::string annualFolder = it != folderByAnnualId.end()
        ? it->second
        : safeFilePart(item.annualId);
    return baseOutput / annualFolder / (fileStem + "." + ext);
}

} // namespace

namespace core::exporting {

ExportResult AnalysisExportService::exportAnalyses(const ExportRequest& request) const
{
    ExportResult result;
    result.actualFormat = request.format;
    result.resolvedOutputPath = request.outputPath;

    if (request.outputPath.empty()) {
        result.status = ExportStatus::InvalidInput;
        result.errorCode = std::string(core::constants::exportFlow::errors::kOutputPathEmpty);
        result.message = std::string(core::constants::exportFlow::messages::kOutputPathEmpty);
        return result;
    }
    if (!request.stateSnapshot) {
        result.status = ExportStatus::InvalidInput;
        result.errorCode = std::string(core::constants::exportFlow::errors::kStateMissing);
        result.message = std::string(core::constants::exportFlow::messages::kStateMissing);
        return result;
    }

    try {
        std::filesystem::path baseOutput = std::filesystem::path(request.outputPath);
        const bool hasExtension = baseOutput.has_extension();
        if (hasExtension) {
            baseOutput = baseOutput.parent_path() / baseOutput.stem();
        }

        if (std::filesystem::exists(baseOutput)) {
            std::filesystem::remove_all(baseOutput);
        }
        std::filesystem::create_directories(baseOutput);

        const auto folderByAnnualId = annualFolderNames(*request.stateSnapshot);

        core::application::AnalysisService analysisService;
        for (size_t i = 0; i < request.analysisItems.size(); ++i) {
            const auto& item = request.analysisItems[i];
            const auto computed = analysisService.computeAnalysisById(*request.stateSnapshot, item.analysisId);
            if (!computed.found) continue;

            const std::filesystem::path outputFile = outputPathForItem(baseOutput, folderByAnnualId, item);
            std::filesystem::create_directories(outputFile.parent_path());

            bool ok = false;
            const auto tableRows = normalizedRowsForExport(computed, *request.stateSnapshot);
            switch (item.format) {
            case AnalysisExportFormat::Csv:
                ok = writeCsvTable(outputFile, tableRows);
                break;
            case AnalysisExportFormat::Xlsx:
                ok = writePseudoXlsxTable(outputFile, tableRows);
                break;
            case AnalysisExportFormat::Jpg:
            case AnalysisExportFormat::Png:
                ok = writeImageFromResult(outputFile, computed.type.empty() ? item.analysisId : computed.type, computed);
                break;
            }

            if (!ok) {
                result.status = ExportStatus::WriteFailed;
                result.errorCode = std::string(core::constants::exportFlow::errors::kFileWriteFailed);
                result.message = std::string(core::constants::exportFlow::messages::kFileWriteFailed);
                return result;
            }
        }

        if (request.packageFormat == PackageFormat::Zip) {
            const std::filesystem::path archivePath = baseOutput.string() + std::string(core::constants::exportFlow::packaging::kZipExtension);
            if (!createZipArchive(baseOutput, archivePath)) {
                result.status = ExportStatus::ArchiveFailed;
                result.errorCode = std::string(core::constants::exportFlow::errors::kArchiveFailed);
                result.message = std::string(core::constants::exportFlow::messages::kArchiveFailed);
                return result;
            }
            result.resolvedOutputPath = archivePath.string();
        }

        result.status = ExportStatus::Ok;
        return result;
    } catch (...) {
        result.status = ExportStatus::InternalError;
        result.errorCode = std::string(core::constants::exportFlow::errors::kInternalError);
        result.message = std::string(core::constants::exportFlow::messages::kInternalError);
        return result;
    }
}

} // namespace core::exporting
