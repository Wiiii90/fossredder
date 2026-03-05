#include "core/controllers/CsvController.h"

#include "core/errors/ErrorReporterRegistry.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/models/AppState.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"
#include <algorithm>
#include <cctype>

namespace core::controllers::exporting {

static std::string escapeCsvField(const std::string& s, char sep) {
    bool needQuote = false;
    for (char c : s) {
        if (c == '"' || c == '\n' || c == '\r' || c == sep) { needQuote = true; break; }
    }
    if (!needQuote) return s;
    std::string out;
    out.push_back('"');
    for (char c : s) {
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

bool CsvController::exportData(const ExportOptions& opts) {
    if (opts.outputPath.empty()) return false;
    if (!opts.stateSnapshot) return false;

    const AppState& state = *opts.stateSnapshot;

    // Build helper maps
    std::unordered_map<std::string, std::string> propIdToName;
    propIdToName.reserve(state.properties.size());
    for (const auto& p : state.properties) {
        if (!p) continue;
        propIdToName[p->id] = p->name;
    }

    std::unordered_map<std::string, std::string> contractIdToType;
    contractIdToType.reserve(state.contracts.size());
    for (const auto& c : state.contracts) {
        if (!c) continue;
        contractIdToType[c->id] = c->type;
    }

    // Derive contract types from dataRows (stable order of first appearance)
    std::vector<std::string> contractTypes;
    std::unordered_set<std::string> seenTypes;

    // Build Data rows: vector of tuples (propName, contractType, amount)
    struct DataRow { std::string prop; std::string type; double amount; };
    std::vector<DataRow> dataRows;
    dataRows.reserve(256);
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
        // fallback: search contracts vector (in case map wasn't populated or ids differ in whitespace)
        for (const auto& c : state.contracts) {
            if (!c) continue;
            if (trim(c->id) == cid && !trim(c->type).empty()) return trim(c->type);
        }
        // diagnostic: print missing id once
        static std::unordered_set<std::string> missingLogged;
        if (missingLogged.insert(cid).second) {
            core::errors::report({
                core::errors::ErrorSeverity::Warning,
                "core::CsvController::exportData",
                std::string("missing contractId->type mapping for id='") + cid + "'",
                {}
            });
        }
        return std::string("(Unassigned)");
    };

    for (const auto& tptr : state.transactions) {
        if (!tptr) continue;
        // resolve contract type, but only include transactions that are assigned to at least one property
        if (tptr->propertyIds.empty()) continue; // skip transactions without property
        const std::string contractType = findContractType(tptr->contractId);
        for (const auto& pid : tptr->propertyIds) {
            const std::string propName = propIdToName.count(pid) ? propIdToName[pid] : pid;
            dataRows.push_back({ propName, contractType, tptr->amount });
        }
    }

    // Precompute matrix sums: map prop -> map type -> sum
    std::unordered_map<std::string, std::unordered_map<std::string, double>> matrix;
    for (const auto& r : dataRows) {
        matrix[r.prop][r.type] += r.amount;
    }

    // Build contractTypes in stable order based on dataRows
    for (const auto& r : dataRows) {
        if (seenTypes.insert(r.type).second) contractTypes.push_back(r.type);
    }

    // Diagnostic logging to help understand why types may be empty
    {
        core::errors::report({
            core::errors::ErrorSeverity::Info,
            "core::CsvController::exportData",
            std::string("dataRows=") + std::to_string(dataRows.size()) + " contractTypes=" + std::to_string(contractTypes.size()),
            {}
        });
    }

    // Build unique property list: include properties from state in defined order only if they have data,
    // then append any additional property names found in the matrix.
    std::vector<std::string> propertyList;
    std::unordered_set<std::string> seenProps;
    for (const auto& p : state.properties) {
        if (!p) continue;
        if (matrix.find(p->name) != matrix.end()) {
            if (seenProps.insert(p->name).second) propertyList.push_back(p->name);
        }
    }
    for (const auto& kv : matrix) {
        if (seenProps.insert(kv.first).second) propertyList.push_back(kv.first);
    }

    // Determine decimal separator based on locale (simple heuristic)
    char decimalSep = '.';
    if (!opts.locale.empty()) {
        if (opts.locale.rfind("de", 0) == 0) decimalSep = ','; // German-like
    }

    const char sep = ';';

    std::ofstream out(opts.outputPath, std::ios::binary);
    if (!out) return false;
    // BOM
    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    out.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    auto formatNumber = [&](double v)->std::string {
        std::ostringstream ss;
        ss.setf(std::ios::fixed);
        ss << std::setprecision(2) << v;
        std::string s = ss.str();
        if (decimalSep != '.') {
            for (auto& ch : s) if (ch == '.') ch = decimalSep;
        }
        return s;
    };

    // Write only the Matrix (header + aggregated rows). No separate Data block or extra title.
    // header
    out << "Gebäude";
    for (const auto& ct : contractTypes) out << sep << escapeCsvField(ct.empty() ? std::string("(Unassigned)") : ct, sep);
    out << sep << "Summe" << "\n";

    for (const auto& pname : propertyList) {
        out << escapeCsvField(pname, sep);
        double rowSum = 0.0;
        auto pit = matrix.find(pname);
        for (const auto& ct : contractTypes) {
            double v = 0.0;
            if (pit != matrix.end()) {
                auto tit = pit->second.find(ct.empty() ? std::string("(Unassigned)") : ct);
                if (tit != pit->second.end()) v = tit->second;
            }
            out << sep << formatNumber(v);
            rowSum += v;
        }
        out << sep << formatNumber(rowSum) << "\n";
    }

    out.close();
    return true;
}

} // namespace core::controllers::exporting
