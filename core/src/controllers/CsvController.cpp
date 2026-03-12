/**
 * @file core/src/controllers/CsvController.cpp
 * @brief Implements CSV export for the property/contract-type matrix.
 */
#include "core/controllers/CsvController.h"

#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Transaction.h"
#include "core/utils/Util.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

std::string escapeCsv(const std::string& s, char sep)
{
    bool q = false;
    for (char c : s) if (c == '"' || c == '\n' || c == '\r' || c == sep) { q = true; break; }
    if (!q) return s;
    std::string out;
    out.push_back('"');
    for (char c : s) { if (c == '"') out.push_back('"'); out.push_back(c); }
    out.push_back('"');
    return out;
}

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
    static std::unordered_set<std::string> logged;
    if (logged.insert(cid).second)
        core::errors::report({ core::errors::ErrorSeverity::Warning, "CsvController",
            "missing contractId->type for id='" + cid + "'", {} });
    return std::string(core::constants::exportFlow::labels::kUnassigned);
}

} // namespace

namespace core::controllers::exporting {

ExportResult CsvController::exportData(const ExportRequest& request)
{
    ExportResult result;
    result.actualFormat = ExportFormat::Csv;
    result.resolvedOutputPath = request.outputPath;

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
    rows.reserve(256);

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

    char decSep = '.';
    if (!request.locale.empty() && request.locale.rfind("de", 0) == 0) decSep = ',';
    constexpr char sep = ';';

    std::ofstream out(request.outputPath, std::ios::binary);
    if (!out) {
        result.status    = ExportStatus::WriteFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kFileOpenFailed);
        result.message   = std::string(core::constants::exportFlow::messages::kFileOpenFailed);
        return result;
    }

    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    out.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    auto fmt = [&](double v) {
        std::ostringstream ss; ss << std::fixed << std::setprecision(2) << v;
        std::string s = ss.str();
        if (decSep != '.') for (auto& c : s) if (c == '.') c = decSep;
        return s;
    };

    out << std::string(core::constants::exportFlow::labels::kPropertyHeader);
    for (const auto& ct : contractTypes) out << sep << escapeCsv(ct, sep);
    out << sep << std::string(core::constants::exportFlow::labels::kTotal) << "\n";

    for (const auto& pname : propList) {
        out << escapeCsv(pname, sep);
        double rowSum = 0.0;
        const auto pit = matrix.find(pname);
        for (const auto& ct : contractTypes) {
            double v = 0.0;
            if (pit != matrix.end()) { const auto tit = pit->second.find(ct); if (tit != pit->second.end()) v = tit->second; }
            out << sep << fmt(v); rowSum += v;
        }
        out << sep << fmt(rowSum) << "\n";
    }

    out.close();
    if (!out) {
        result.status    = ExportStatus::WriteFailed;
        result.errorCode = std::string(core::constants::exportFlow::errors::kFileWriteFailed);
        result.message   = std::string(core::constants::exportFlow::messages::kFileWriteFailed);
        return result;
    }

    result.status = ExportStatus::Ok;
    return result;
}

} // namespace core::controllers::exporting
