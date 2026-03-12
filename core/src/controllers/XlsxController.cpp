/**
 * @file core/src/controllers/XlsxController.cpp
 * @brief Implements XLSX export for the property/contract-type matrix.
 */
#include "core/controllers/XlsxController.h"

#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Transaction.h"
#include "core/utils/Util.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <xlnt/xlnt.hpp>

namespace {

constexpr auto kWorksheetTitle = "Export";

std::string resolveContractType(const std::string& contractId,
                                const std::unordered_map<std::string, std::string>& idToType,
                                const AppState& state)
{
    const std::string cid = utils::trim(contractId);
    if (cid.empty()) return std::string(core::constants::exportFlow::labels::kUnassigned);
    const auto it = idToType.find(cid);
    if (it != idToType.end() && !utils::trim(it->second).empty()) return utils::trim(it->second);
    for (const auto& c : state.contracts) {
        if (!c) continue;
        if (utils::trim(c->id) == cid && !utils::trim(c->type).empty()) return utils::trim(c->type);
    }
    return std::string(core::constants::exportFlow::labels::kUnassigned);
}

} // namespace

namespace core::controllers::exporting {

ExportResult XlsxController::exportData(const ExportRequest& request)
{
    ExportResult result;
    result.actualFormat       = ExportFormat::Xlsx;
    result.resolvedOutputPath = request.outputPath;

    try {
        if (request.outputPath.empty()) {
            result.status    = ExportStatus::InvalidInput;
            result.errorCode = std::string(core::constants::exportFlow::errors::kOutputPathEmpty);
            result.message   = std::string(core::constants::exportFlow::messages::kOutputPathEmpty);
            return result;
        }
        if (!request.stateSnapshot) {
            result.status    = ExportStatus::InvalidInput;
            result.errorCode = std::string(core::constants::exportFlow::errors::kStateMissing);
            result.message   = std::string(core::constants::exportFlow::messages::kStateMissing);
            return result;
        }

        const AppState& state = *request.stateSnapshot;

        std::unordered_map<std::string, std::string> propIdToName;
        for (const auto& p : state.properties) if (p) propIdToName[p->id] = p->name;
        std::unordered_map<std::string, std::string> contractIdToType;
        for (const auto& c : state.contracts) if (c) contractIdToType[c->id] = c->type;

        struct Row { std::string prop; std::string type; double amount; };
        std::vector<Row> rows;
        rows.reserve(512);

        for (const auto& t : state.transactions) {
            if (!t || t->propertyIds.empty()) continue;
            const std::string ct = resolveContractType(t->contractId, contractIdToType, state);
            for (const auto& pid : t->propertyIds) {
                const auto it = propIdToName.find(pid);
                rows.push_back({ it != propIdToName.end() ? it->second : pid, ct, t->amount });
            }
        }

        std::unordered_map<std::string, std::unordered_map<std::string, double>> matrix;
        for (const auto& r : rows) matrix[r.prop][r.type] += r.amount;

        std::vector<std::string> contractTypes;
        std::unordered_set<std::string> seenTypes;
        for (const auto& r : rows) if (seenTypes.insert(r.type).second) contractTypes.push_back(r.type);

        std::vector<std::string> propList;
        std::unordered_set<std::string> seenProps;
        for (const auto& p : state.properties) {
            if (!p || !matrix.count(p->name)) continue;
            if (seenProps.insert(p->name).second) propList.push_back(p->name);
        }
        for (const auto& kv : matrix) if (seenProps.insert(kv.first).second) propList.push_back(kv.first);

        xlnt::workbook wb;
        xlnt::worksheet ws = wb.active_sheet();
        ws.title(kWorksheetTitle);

        ws.cell(1, 1).value(std::string(core::constants::exportFlow::labels::kPropertyHeader));
        for (size_t j = 0; j < propList.size(); ++j)
            ws.cell(1, static_cast<int>(2 + j)).value(propList[j]);
        ws.cell(1, static_cast<int>(2 + propList.size())).value(
            std::string(core::constants::exportFlow::labels::kTotal));

        int row = 2;
        std::vector<double> colSums(propList.size(), 0.0);
        for (const auto& ct : contractTypes) {
            ws.cell(row, 1).value(ct);
            double rowSum = 0.0;
            for (size_t j = 0; j < propList.size(); ++j) {
                double v = 0.0;
                const auto pit = matrix.find(propList[j]);
                if (pit != matrix.end()) { const auto tit = pit->second.find(ct); if (tit != pit->second.end()) v = tit->second; }
                ws.cell(row, static_cast<int>(2 + j)).value(v);
                rowSum += v; colSums[j] += v;
            }
            ws.cell(row, static_cast<int>(2 + propList.size())).value(rowSum);
            ++row;
        }
        if (!propList.empty()) {
            ws.cell(row, 1).value(std::string(core::constants::exportFlow::labels::kTotal));
            double grand = 0.0;
            for (size_t j = 0; j < propList.size(); ++j) {
                ws.cell(row, static_cast<int>(2 + j)).value(colSums[j]);
                grand += colSums[j];
            }
            ws.cell(row, static_cast<int>(2 + propList.size())).value(grand);
        }

        wb.save(request.outputPath);
        result.status = ExportStatus::Ok;

    } catch (const std::exception&) {
        core::errors::reportException(core::errors::ErrorSeverity::Error,
            "XlsxController::exportData", std::current_exception());
        result.status    = ExportStatus::XlsxGenerationFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kXlsxGenerationFailed);
        result.message   = std::string(core::constants::exportFlow::messages::kXlsxGenerationFailed);
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Error,
            "XlsxController::exportData", std::current_exception());
        result.status    = ExportStatus::InternalError;
        result.errorCode = std::string(core::constants::exportFlow::errors::kInternalError);
        result.message   = std::string(core::constants::exportFlow::messages::kInternalError);
    }
    return result;
}

} // namespace core::controllers::exporting
