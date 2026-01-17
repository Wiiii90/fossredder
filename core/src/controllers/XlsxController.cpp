#include "core/controllers/XlsxController.h"

#include <iostream>
#include <stdexcept>
#include <xlnt/xlnt.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>

#include "core/models/AppState.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

namespace core::controllers::exporting {

bool XlsxController::exportData(const ExportOptions& opts) {
    try {
        if (opts.outputPath.empty()) return false;

        xlnt::workbook wb;

        // Validate AppState
        if (!opts.state) return false;
        const AppState& state = *opts.state;

        // Collect properties map id->name for lookups
        std::unordered_map<std::string, std::string> propIdToName;
        propIdToName.reserve(state.properties.size());
        for (const auto& p : state.properties) {
            if (!p) continue;
            propIdToName[p->id] = p->name;
        }

        // Collect contract id->type map
        std::unordered_map<std::string, std::string> contractIdToType;
        contractIdToType.reserve(state.contracts.size());
        for (const auto& c : state.contracts) {
            if (!c) continue;
            contractIdToType[c->id] = c->type;
        }

        // Matrix sheet: properties vs contract types
        // Collect distinct contract types in stable order (appearance order of contracts)
        std::vector<std::string> contractTypes;
        std::unordered_set<std::string> seenTypes;
        for (const auto& c : state.contracts) {
            if (!c) continue;
            const std::string& ct = c->type;
            if (ct.empty()) continue;
            if (seenTypes.insert(ct).second) contractTypes.push_back(ct);
        }

        // Also include contractTypes referenced by transactions even if contract not in list
        for (const auto& tptr : state.transactions) {
            if (!tptr) continue;
            if (tptr->contractId.empty()) continue;
            const auto it = contractIdToType.find(tptr->contractId);
            if (it == contractIdToType.end()) continue;
            const std::string& ct = it->second;
            if (ct.empty()) continue;
            if (seenTypes.insert(ct).second) contractTypes.push_back(ct);
        }

        // Build aggregated data rows similar to CSV exporter
        struct DataRow { std::string prop; std::string type; double amount; };
        std::vector<DataRow> dataRows;
        dataRows.reserve(512);

        auto trim = [](std::string s)->std::string {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
            return s;
        };

        auto findContractType = [&](const std::string& cid_in)->std::string {
            const std::string cid = trim(cid_in);
            if (cid.empty()) return std::string("(Unassigned)");
            auto it = contractIdToType.find(cid);
            if (it != contractIdToType.end() && !trim(it->second).empty()) return trim(it->second);
            for (const auto& c : state.contracts) {
                if (!c) continue;
                if (trim(c->id) == cid && !trim(c->type).empty()) return trim(c->type);
            }
            return std::string("(Unassigned)");
        };

        for (const auto& tptr : state.transactions) {
            if (!tptr) continue;
            if (tptr->propertyIds.empty()) continue;
            std::string contractType = findContractType(tptr->contractId);
            for (const auto& pid : tptr->propertyIds) {
                const std::string propName = propIdToName.count(pid) ? propIdToName[pid] : pid;
                dataRows.push_back({ propName, contractType, tptr->amount });
            }
        }

        // Precompute matrix sums: prop -> type -> sum
        std::unordered_map<std::string, std::unordered_map<std::string, double>> matrix;
        for (const auto& r : dataRows) matrix[r.prop][r.type] += r.amount;

        // Derive contractTypes by first appearance in dataRows
        std::vector<std::string> contractTypesByData;
        std::unordered_set<std::string> seenTypesByData;
        for (const auto& r : dataRows) if (seenTypesByData.insert(r.type).second) contractTypesByData.push_back(r.type);

        // Build unique property list: prefer state.properties order but only include those with data
        std::vector<std::string> propertyList;
        std::unordered_set<std::string> seenProps;
        for (const auto& p : state.properties) {
            if (!p) continue;
            if (matrix.find(p->name) != matrix.end()) { seenProps.insert(p->name); propertyList.push_back(p->name); }
        }
        for (const auto& kv : matrix) if (seenProps.insert(kv.first).second) propertyList.push_back(kv.first);

        // write matrix sheet
        xlnt::worksheet wsMatrix = wb.active_sheet();
        wsMatrix.title("Export");

        // Transposed layout: header row lists properties, rows represent contract types.
        // Header: first cell = "Gebäude", then one column per property, then final "Summe" column.
        wsMatrix.cell(1, 1).value("Gebäude");
        for (size_t j = 0; j < propertyList.size(); ++j) {
            wsMatrix.cell(1, static_cast<int>(2 + j)).value(propertyList[j]);
        }
        wsMatrix.cell(1, static_cast<int>(2 + propertyList.size())).value("Summe");

        // Write one row per contract type and accumulate column sums
        int typeRow = 2;
        std::vector<double> colSums(propertyList.size(), 0.0);
        double grandTotal = 0.0;
        for (size_t i = 0; i < contractTypesByData.size(); ++i) {
            const auto& ctype = contractTypesByData[i];
            wsMatrix.cell(typeRow, 1).value(ctype.empty() ? std::string("(Unassigned)") : ctype);
            double rowSum = 0.0;
            for (size_t j = 0; j < propertyList.size(); ++j) {
                double v = 0.0;
                const auto pit = matrix.find(propertyList[j]);
                if (pit != matrix.end()) {
                    auto tit = pit->second.find(ctype);
                    if (tit != pit->second.end()) v = tit->second;
                }
                wsMatrix.cell(typeRow, static_cast<int>(2 + j)).value(v);
                rowSum += v;
                colSums[j] += v;
            }
            wsMatrix.cell(typeRow, static_cast<int>(2 + propertyList.size())).value(rowSum);
            grandTotal += rowSum;
            ++typeRow;
        }

        // Totals row: sums per property and grand total
        if (!propertyList.empty()) {
            wsMatrix.cell(typeRow, 1).value("Summe");
            double totalRow = 0.0;
            for (size_t j = 0; j < propertyList.size(); ++j) {
                wsMatrix.cell(typeRow, static_cast<int>(2 + j)).value(colSums[j]);
                totalRow += colSums[j];
            }
            wsMatrix.cell(typeRow, static_cast<int>(2 + propertyList.size())).value(totalRow);
        }

        wb.save(opts.outputPath);
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "XlsxController::exportData failed: " << ex.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "XlsxController::exportData failed: unknown error" << std::endl;
        return false;
    }
}

} // namespace core::controllers::exporting
