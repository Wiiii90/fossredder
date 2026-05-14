/**
 * @file core/src/export/AnalysisExportService.cpp
 * @brief Implements analysis export to CSV/XLSX/JPG/PNG plus optional packaging.
 */

#include "core/application/export/AnalysisExportService.h"

#include "core/application/analysis/AnalysisService.h"
#include "core/constants/analysis.h"
#include "core/constants/export.h"
#include "core/constants/filters.h"
#include "core/ports/archive/IArchive.h"
#include "core/ports/analysis-image-renderer/IAnalysisImageRenderer.h"
#include "core/ports/xlsx-writer/IXlsxWriter.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/domain/catalog/WorkspaceCatalog.h"

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

std::string extensionFor(core::application::exporting::AnalysisExportFormat format)
{
    using core::application::exporting::AnalysisExportFormat;
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

std::unordered_map<std::string, std::string> propertyNameById(const core::domain::catalog::WorkspaceCatalog& state)
{
    std::unordered_map<std::string, std::string> out;
    out.reserve(state.properties().size());
    for (const auto& property : state.properties()) {
        if (!property || property->id().empty()) continue;
        out[property->id()] = property->name().empty() ? property->id() : property->name();
    }
    return out;
}

std::vector<std::vector<std::string>> normalizedRowsForExport(const core::domain::AnalysisResult& result,
                                                              const core::domain::catalog::WorkspaceCatalog& state)
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
    rows.push_back({std::string(core::constants::exportFlow::labels::kPropertyHeader),
                    std::string(core::constants::filters::kContractType),
                    std::string(core::constants::exportFlow::labels::kTotal)});

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

std::unordered_map<std::string, std::string> annualFolderNames(const core::domain::catalog::WorkspaceCatalog& state)
{
    std::unordered_map<std::string, std::string> folderByAnnualId;
    std::set<std::string> usedFolderNames;

    for (const auto& annual : state.annuals()) {
        if (!annual || annual->id().empty()) continue;

        const std::string baseName = safeFilePart(!annual->name().empty() ? annual->name() : annual->id());
        std::string candidate = baseName;
        if (usedFolderNames.find(candidate) != usedFolderNames.end()) {
            std::string suffix = annual->id();
            if (suffix.size() > 8) suffix = suffix.substr(0, 8);
            candidate = baseName + "_" + safeFilePart(suffix);
        }

        int idx = 2;
        while (usedFolderNames.find(candidate) != usedFolderNames.end()) {
            candidate = baseName + "_" + std::to_string(idx++);
        }

        usedFolderNames.insert(candidate);
        folderByAnnualId[annual->id()] = candidate;
    }

    return folderByAnnualId;
}

std::filesystem::path outputPathForItem(const std::filesystem::path& baseOutput,
                                        const std::unordered_map<std::string, std::string>& folderByAnnualId,
                                        const core::application::exporting::AnalysisExportItem& item)
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

namespace core::application::exporting {

AnalysisExportService::AnalysisExportService(std::shared_ptr<core::ports::archive::IArchive> archive,
                                             std::shared_ptr<core::ports::xlsx_writer::IXlsxWriter> xlsxWriter,
                                             std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer)
    : archive_(std::move(archive)),
      xlsxWriter_(std::move(xlsxWriter)),
      imageRenderer_(std::move(imageRenderer)) {
}

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

        core::application::analysis::AnalysisService analysisService;
        for (size_t i = 0; i < request.analysisItems.size(); ++i) {
            const auto& item = request.analysisItems[i];
            const auto computed = analysisService.runAnalysisById(*request.stateSnapshot, item.analysisId);
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
                ok = xlsxWriter_ && xlsxWriter_->writeTable(outputFile, tableRows, "Analysis");
                break;
            case AnalysisExportFormat::Jpg:
            case AnalysisExportFormat::Png:
                ok = imageRenderer_ && imageRenderer_->writeAnalysisImage(outputFile, computed.type.empty() ? item.analysisId : computed.type, computed);
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
            if (!archive_ || !archive_->create(baseOutput, archivePath, request.packageFormat)) {
                result.status = ExportStatus::ArchiveFailed;
                result.errorCode = std::string(core::constants::exportFlow::errors::kArchiveFailed);
                result.message = std::string(core::constants::exportFlow::messages::kArchiveFailed);
                return result;
            }
            result.resolvedOutputPath = archivePath.string();
        }

        result.status = ExportStatus::Ok;
        result.success = true;
        return result;
    } catch (...) {
        result.status = ExportStatus::InternalError;
        result.errorCode = std::string(core::constants::exportFlow::errors::kInternalError);
        result.message = std::string(core::constants::exportFlow::messages::kInternalError);
        return result;
    }
}

} // namespace core::application::exporting
