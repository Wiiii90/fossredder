/**
 * @file core/src/export/XlsxExporter.cpp
 * @brief Implements XLSX export for the property/contract-type matrix.
 */

#include "core/export/XlsxExporter.h"

#include "PropertyContractMatrix.h"
#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <vector>

#include <xlnt/xlnt.hpp>

namespace {

constexpr auto kWorksheetTitle = "Export";

} // namespace

namespace core::exporting {

ExportResult XlsxExporter::exportData(const ExportRequest& request) const
{
    ExportResult result;
    result.actualFormat = ExportFormat::Xlsx;
    result.resolvedOutputPath = request.outputPath;

    try {
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

        const auto matrix = detail::buildPropertyContractMatrix(*request.stateSnapshot,
                                                                "core::exporting::XlsxExporter::exportData");

        xlnt::workbook workbook;
        xlnt::worksheet worksheet = workbook.active_sheet();
        worksheet.title(kWorksheetTitle);

        worksheet.cell(1, 1).value(std::string(core::constants::exportFlow::labels::kPropertyHeader));
        for (size_t column = 0; column < matrix.propertyNames.size(); ++column) {
            worksheet.cell(1, static_cast<int>(2 + column)).value(matrix.propertyNames[column]);
        }
        worksheet.cell(1, static_cast<int>(2 + matrix.propertyNames.size())).value(
            std::string(core::constants::exportFlow::labels::kTotal));

        int row = 2;
        std::vector<double> columnSums(matrix.propertyNames.size(), 0.0);
        for (const auto& contractType : matrix.contractTypes) {
            worksheet.cell(row, 1).value(contractType);
            double rowSum = 0.0;
            for (size_t column = 0; column < matrix.propertyNames.size(); ++column) {
                double value = 0.0;
                const auto propertyIt = matrix.amountsByProperty.find(matrix.propertyNames[column]);
                if (propertyIt != matrix.amountsByProperty.end()) {
                    const auto contractIt = propertyIt->second.find(contractType);
                    if (contractIt != propertyIt->second.end()) {
                        value = contractIt->second;
                    }
                }
                worksheet.cell(row, static_cast<int>(2 + column)).value(value);
                rowSum += value;
                columnSums[column] += value;
            }
            worksheet.cell(row, static_cast<int>(2 + matrix.propertyNames.size())).value(rowSum);
            ++row;
        }

        if (!matrix.propertyNames.empty()) {
            worksheet.cell(row, 1).value(std::string(core::constants::exportFlow::labels::kTotal));
            double grandTotal = 0.0;
            for (size_t column = 0; column < matrix.propertyNames.size(); ++column) {
                worksheet.cell(row, static_cast<int>(2 + column)).value(columnSums[column]);
                grandTotal += columnSums[column];
            }
            worksheet.cell(row, static_cast<int>(2 + matrix.propertyNames.size())).value(grandTotal);
        }

        workbook.save(request.outputPath);
        result.status = ExportStatus::Ok;
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Error,
                                      "core::exporting::XlsxExporter::exportData",
                                      std::current_exception());
        result.status = ExportStatus::XlsxGenerationFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kXlsxGenerationFailed);
        result.message = std::string(core::constants::exportFlow::messages::kXlsxGenerationFailed);
    }

    return result;
}

} // namespace core::exporting
