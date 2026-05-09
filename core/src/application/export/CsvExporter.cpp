/**
 * @file core/src/export/CsvExporter.cpp
 * @brief Implements CSV export for the property/contract-type matrix.
 */

#include "core/application/export/CsvExporter.h"

#include "PropertyContractMatrix.h"
#include "core/constants/export.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string escapeCsv(const std::string& value, char separator)
{
    bool requiresQuoting = false;
    for (const char c : value) {
        if (c == '"' || c == '\n' || c == '\r' || c == separator) {
            requiresQuoting = true;
            break;
        }
    }
    if (!requiresQuoting) {
        return value;
    }

    std::string escaped;
    escaped.push_back('"');
    for (const char c : value) {
        if (c == '"') {
            escaped.push_back('"');
        }
        escaped.push_back(c);
    }
    escaped.push_back('"');
    return escaped;
}

} // namespace

namespace core::application::exporting {

ExportResult CsvExporter::exportData(const ExportRequest& request) const
{
    ExportResult result;
    result.actualFormat = ExportFormat::Csv;
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

    const auto matrix = internal::buildPropertyContractMatrix(*request.stateSnapshot,
                                                            "core::exporting::CsvExporter::exportData");

    char decimalSeparator = '.';
    if (!request.locale.empty() && request.locale.rfind("de", 0) == 0) {
        decimalSeparator = ',';
    }
    constexpr char separator = ';';

    std::ofstream output(request.outputPath, std::ios::binary);
    if (!output) {
        result.status = ExportStatus::WriteFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kFileOpenFailed);
        result.message = std::string(core::constants::exportFlow::messages::kFileOpenFailed);
        return result;
    }

    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    output.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    const auto formatAmount = [&](double value) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        std::string text = stream.str();
        if (decimalSeparator != '.') {
            for (auto& c : text) {
                if (c == '.') {
                    c = decimalSeparator;
                }
            }
        }
        return text;
    };

    output << std::string(core::constants::exportFlow::labels::kPropertyHeader);
    for (const auto& contractType : matrix.contractTypes) {
        output << separator << escapeCsv(contractType, separator);
    }
    output << separator << std::string(core::constants::exportFlow::labels::kTotal) << "\n";

    for (const auto& propertyName : matrix.propertyNames) {
        output << escapeCsv(propertyName, separator);
        double rowSum = 0.0;
        const auto propertyIt = matrix.amountsByProperty.find(propertyName);
        for (const auto& contractType : matrix.contractTypes) {
            double value = 0.0;
            if (propertyIt != matrix.amountsByProperty.end()) {
                const auto contractIt = propertyIt->second.find(contractType);
                if (contractIt != propertyIt->second.end()) {
                    value = contractIt->second;
                }
            }
            output << separator << formatAmount(value);
            rowSum += value;
        }
        output << separator << formatAmount(rowSum) << "\n";
    }

    output.close();
    if (!output) {
        result.status = ExportStatus::WriteFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kFileWriteFailed);
        result.message = std::string(core::constants::exportFlow::messages::kFileWriteFailed);
        return result;
    }

    result.status = ExportStatus::Ok;
    result.success = true;
    return result;
}

} // namespace core::exporting
