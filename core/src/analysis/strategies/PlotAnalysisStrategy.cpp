#include "core/analysis/strategies/PlotAnalysisStrategy.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/models/Transaction.h"
#include "core/models/Contract.h"
#include "core/analysis/Filter.h"
#include <map>
#include <sstream>
#include <cmath>
#include <nlohmann/json.hpp>

AnalysisResult PlotAnalysisStrategy::compute(const Analysis& analysis, const AppState& state, const std::string& filterSpec) const {
    AnalysisResult res;
    res.generatedAt = "";

    // very simple demo implementation: support "pie" and "histogram" as subtype in analysis.type
    // subtype may be encoded in analysis.configJson (e.g. "pie"/"histogram")
    std::string subtype = analysis.type;
    // parse optional config JSON from analysis.configJson to determine plotMeasure and properties
    std::string plotType = subtype;
    std::string plotMeasure = "Total Amount";
    std::vector<std::string> propertyFilter;
    std::vector<std::string> contractTypeFilter;
    try {
        if (!analysis.configJson.empty()) {
            auto j = nlohmann::json::parse(analysis.configJson);
            if (j.contains("plotType")) plotType = j["plotType"].get<std::string>();
            if (j.contains("plotMeasure")) plotMeasure = j["plotMeasure"].get<std::string>();
            if (j.contains("properties") && j["properties"].is_array()) {
                for (const auto& p : j["properties"]) propertyFilter.push_back(p.get<std::string>());
            }
            if (j.contains("contractTypes") && j["contractTypes"].is_array()) {
                for (const auto& ct : j["contractTypes"]) contractTypeFilter.push_back(ct.get<std::string>());
            }
        }
    } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::analysis::PlotAnalysisStrategy::parseConfig", std::current_exception()); }

    if (!propertyFilter.empty()) {
        core::errors::report({
            core::errors::ErrorSeverity::Info,
            "core::analysis::PlotAnalysisStrategy::compute",
            std::string("propertyFilter size=") + std::to_string(propertyFilter.size()),
            {}
        });
        for (const auto& pf : propertyFilter) {
            core::errors::report({
                core::errors::ErrorSeverity::Info,
                "core::analysis::PlotAnalysisStrategy::compute",
                std::string("propertyFilter value=") + pf,
                {}
            });
        }
    }

    if (plotType == "pie") {
        // aggregate by contract.type (or "unassigned") using selected properties if provided
        std::map<std::string, double> agg;
        std::map<std::string, int> count;
        // build contractId -> contract.type map (normalized lowercase trimmed) for reliable comparisons
        std::map<std::string, std::string> contractTypeById;
        auto normalize = [](const std::string &s)->std::string {
            std::string out;
            // trim
            size_t a = 0; while (a < s.size() && std::isspace((unsigned char)s[a])) ++a;
            size_t b = s.size(); while (b > a && std::isspace((unsigned char)s[b-1])) --b;
            if (b <= a) return std::string();
            out = s.substr(a, b-a);
            // lowercase
            for (auto &ch : out) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            return out;
        };
        for (const auto& cptr : state.contracts) {
            if (!cptr) continue;
            contractTypeById[cptr->id] = normalize(cptr->type);
        }
        // parse generic filter spec and apply it per-transaction so strategies can respect it
        Filter f = parseFilterSpec(filterSpec);
        for (const auto& tptr : state.transactions) {
            if (!tptr) continue;
            // apply filterSpec if provided
            if (!filterSpec.empty() && !f.matches(tptr, state)) continue;
            // apply property filter if present
            if (!propertyFilter.empty()) {
                bool any = false;
                for (const auto& pid : tptr->propertyIds) {
                    for (const auto& fp : propertyFilter) if (pid == fp) { any = true; break; }
                    if (any) break;
                }
                if (!any) continue;
            }
            // apply contract type filter if present (normalize comparisons)
            if (!contractTypeFilter.empty()) {
                bool ok = false;
                // build normalized wanted set once
                static std::vector<std::string> wantedNorm; wantedNorm.clear();
                for (const auto& ct : contractTypeFilter) {
                    // normalize contract type tokens
                    std::string tmp = ct;
                    // trim
                    size_t a = 0; while (a < tmp.size() && std::isspace((unsigned char)tmp[a])) ++a;
                    size_t b = tmp.size(); while (b > a && std::isspace((unsigned char)tmp[b-1])) --b;
                    if (b <= a) continue;
                    std::string sub = tmp.substr(a, b-a);
                    for (auto &ch : sub) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    wantedNorm.push_back(sub);
                }
                if (tptr->contractId.empty()) {
                    for (const auto& w : wantedNorm) if (w == "unassigned") { ok = true; break; }
                } else {
                    auto it = contractTypeById.find(tptr->contractId);
                    if (it != contractTypeById.end()) {
                        for (const auto& w : wantedNorm) if (it->second == w) { ok = true; break; }
                    }
                }
                if (!ok) continue;
            }
            std::string key = "unassigned";
            if (!tptr->contractId.empty()) {
                auto it = contractTypeById.find(tptr->contractId);
                if (it != contractTypeById.end() && !it->second.empty()) key = it->second; else key = tptr->contractId;
            }
            agg[key] += tptr->amount;
            count[key]++;
        }
        // convert to table rows [label,value]
        double totalAll = 0.0; size_t totalCount = 0;
        for (const auto& kv : agg) { totalAll += std::fabs(kv.second); totalCount += count[kv.first]; }
        for (const auto& kv : agg) {
            double raw = kv.second;
            double val = 0.0;
            if (plotMeasure == "Count") {
                val = static_cast<double>(count[kv.first]);
            } else if (plotMeasure == "Average Amount") {
                val = (count[kv.first] > 0) ? (std::fabs(kv.second) / count[kv.first]) : 0.0;
            } else {
                // Total Amount: use absolute magnitude so pie slices reflect magnitude regardless of sign
                val = std::fabs(kv.second);
            }
            res.table.push_back({ kv.first, std::to_string(val) });
        }
        // simple metrics
        res.metrics["totalAmount"] = totalAll;
        res.metrics["rowCount"] = static_cast<double>(totalCount);
    } else if (plotType == "histogram") {
        // monthly histogram with breakdown by contract type and by property
        // determine matching transactions first (respecting filterSpec and property/contractType filters)
        Filter f = parseFilterSpec(filterSpec);
        struct TxRef { std::shared_ptr<Transaction> t; };
        std::vector<std::shared_ptr<Transaction>> matched;
        for (const auto& tptr : state.transactions) {
            if (!tptr) continue;
            if (!filterSpec.empty() && !f.matches(tptr, state)) continue;
            // apply property filter if present (must have at least one)
            if (!propertyFilter.empty()) {
                bool any = false;
                for (const auto& pid : tptr->propertyIds) {
                    for (const auto& fp : propertyFilter) if (pid == fp) { any = true; break; }
                    if (any) break;
                }
                if (!any) continue;
            }
            // apply contract type filter if present
            if (!contractTypeFilter.empty()) {
                bool ok = false;
                for (const auto& ct : contractTypeFilter) {
                    if (ct == "unassigned" && tptr->contractId.empty()) { ok = true; break; }
                }
                if (!ok && !tptr->contractId.empty()) {
                    // check contract type text
                    for (const auto& cptr : state.contracts) {
                        if (!cptr) continue;
                        if (cptr->id == tptr->contractId) {
                            for (const auto& ct : contractTypeFilter) {
                                std::string a = cptr->type; std::string b = ct;
                                // trim/lower
                                auto norm = [](const std::string &s){ std::string o=s; size_t a=0; while(a<o.size() && isspace((unsigned char)o[a])) ++a; size_t bb=o.size(); while(bb>a && isspace((unsigned char)o[bb-1])) --bb; if (bb<=a) return std::string(); std::string r=o.substr(a,bb-a); for(auto &ch:r) ch=static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); return r; };
                                if (norm(a) == norm(b)) { ok = true; break; }
                            }
                            break;
                        }
                    }
                }
                if (!ok) continue;
            }
            matched.push_back(tptr);
        }

        if (matched.empty()) return res;

        // helper: parse YYYY-MM from bookingDate
        auto yearMonth = [](const std::string &d)->std::string {
            if (d.empty()) return std::string();
            // trim
            size_t a = 0; while (a < d.size() && isspace((unsigned char)d[a])) ++a;
            size_t b = d.size(); while (b > a && isspace((unsigned char)d[b-1])) --b;
            if (b <= a) return std::string();
            std::string s = d.substr(a, b-a);
            // handle YYYY-MM-DD or YYYYMMDD
            if (s.size() >= 7 && isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) && isdigit((unsigned char)s[2]) && isdigit((unsigned char)s[3])) {
                std::string y = s.substr(0,4);
                if (s.size() >= 7 && s[4] == '-') {
                    std::string m = s.substr(5,2);
                    return y + "-" + m;
                }
                if (s.size() >= 6) {
                    std::string m = s.substr(4,2);
                    return y + "-" + m;
                }
            }
            // handle DD.MM.YYYY -> convert to YYYY-MM
            if (s.find('.') != std::string::npos) {
                std::vector<std::string> parts;
                std::istringstream ss(s);
                std::string tok;
                while (std::getline(ss, tok, '.')) parts.push_back(tok);
                if (parts.size() == 3) {
                    std::string dd = parts[0]; std::string mm = parts[1]; std::string yyyy = parts[2];
                    // normalize
                    auto trimLocal = [](std::string str){ size_t a=0; while(a<str.size() && isspace((unsigned char)str[a])) ++a; size_t b=str.size(); while(b>a && isspace((unsigned char)str[b-1])) --b; return str.substr(a,b-a); };
                    dd = trimLocal(dd); mm = trimLocal(mm); yyyy = trimLocal(yyyy);
                    if (yyyy.size() == 4 && (mm.size()==1 || mm.size()==2)) {
                        if (mm.size() == 1) mm = std::string("0") + mm;
                        return yyyy + "-" + mm;
                    }
                }
            }
            // fallback: unknown -> use raw date
            return s;
        };

        // build contractId -> contract.type map (raw type strings)
        std::map<std::string, std::string> contractTypeByIdRaw;
        for (const auto& cptr : state.contracts) { if (!cptr) continue; contractTypeByIdRaw[cptr->id] = cptr->type; }

        // aggregate per month
        std::map<std::string, double> monthTotal;
        std::map<std::string, std::map<std::string, double>> monthByContract;
        std::map<std::string, std::map<std::string, double>> monthByProperty;
        for (const auto& tptr : matched) {
            std::string mon = yearMonth(tptr->bookingDate);
            double amt = std::fabs(tptr->amount);
            monthTotal[mon] += amt;
            // contract type bucket
            std::string ctype = "unassigned";
            if (!tptr->contractId.empty()) {
                auto it = contractTypeByIdRaw.find(tptr->contractId);
                if (it != contractTypeByIdRaw.end() && !it->second.empty()) ctype = it->second; else ctype = tptr->contractId;
            }
            monthByContract[mon][ctype] += amt;
            // property buckets
            if (!tptr->propertyIds.empty()) {
                for (const auto& pid : tptr->propertyIds) monthByProperty[mon][pid] += amt;
            } else {
                monthByProperty[mon]["(no-property)"] += amt;
            }
        }

        // produce ordered months
        std::vector<std::string> months;
        for (const auto& kv : monthTotal) months.push_back(kv.first);
        std::sort(months.begin(), months.end());

        double totalAll = 0.0; // sum of all months
        for (const auto& m : months) {
            nlohmann::json obj;
            obj["month"] = m;
            obj["total"] = monthTotal[m];
            obj["byContract"] = nlohmann::json::object();
            for (const auto& kv : monthByContract[m]) obj["byContract"][kv.first] = kv.second;
            obj["byProperty"] = nlohmann::json::object();
            for (const auto& kv : monthByProperty[m]) obj["byProperty"][kv.first] = kv.second;
            res.table.push_back({ m, obj.dump() });
            totalAll += monthTotal[m];
        }
        // metrics for histogram: total amount and number of months and matched transactions
        res.metrics["totalAmount"] = totalAll;
        res.metrics["rowCount"] = static_cast<double>(months.size());
        res.metrics["matchedTx"] = static_cast<double>(matched.size());
    }

    return res;
}
