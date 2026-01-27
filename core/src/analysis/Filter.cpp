#include "core/analysis/Filter.h"

#include "core/models/AppState.h"
#include "core/models/Transaction.h"
#include "core/models/Contract.h"
#include <memory>

#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

namespace {

static inline string trim(const string& s) {
    size_t a = 0; while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size(); while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

static inline int dateToInt(const string& ymd) {
    // accept YYYY-MM-DD, YYYYMMDD or DD.MM.YYYY input; return integer YYYYMMDD or 0 on parse error
    if (ymd.empty()) return 0;
    string tmp = trim(ymd);
    // strip surrounding quotes if present
    if (tmp.size() >= 2 && ((tmp.front() == '"' && tmp.back() == '"') || (tmp.front() == '\'' && tmp.back() == '\''))) tmp = tmp.substr(1, tmp.size()-2);
    tmp = trim(tmp);

    // If format contains '-', assume YYYY-MM-DD or similar
    if (tmp.find('-') != string::npos) {
        string s = tmp;
        s.erase(remove(s.begin(), s.end(), '-'), s.end());
        if (s.size() == 8) {
            try { return stoi(s); } catch(...) { return 0; }
        }
        return 0;
    }

    // If format contains '.', assume DD.MM.YYYY -> convert to YYYYMMDD
    if (tmp.find('.') != string::npos) {
        // split by '.'
        std::vector<string> parts;
        std::istringstream ss(tmp);
        string tok;
        while (std::getline(ss, tok, '.')) {
            parts.push_back(trim(tok));
        }
        if (parts.size() == 3) {
            // expected DD.MM.YYYY
            string dd = parts[0]; string mm = parts[1]; string yyyy = parts[2];
            if (yyyy.size() == 4 && dd.size() > 0 && mm.size() > 0) {
                // zero-pad day/month
                if (dd.size() == 1) dd = string("0") + dd;
                if (mm.size() == 1) mm = string("0") + mm;
                string out = yyyy + mm + dd;
                try { return stoi(out); } catch(...) { return 0; }
            }
        }
        return 0;
    }

    // If pure digits like YYYYMMDD
    string digits;
    for (char c : tmp) if (isdigit((unsigned char)c)) digits.push_back(c);
    if (digits.size() == 8) {
        try { return stoi(digits); } catch(...) { return 0; }
    }
    return 0;
}

static inline std::string cleanValue(const std::string& s) {
    auto t = trim(s);
    if (t.size() >= 2 && ((t.front() == '"' && t.back() == '"') || (t.front() == '\'' && t.back() == '\''))) t = t.substr(1, t.size()-2);
    // trim again in case quotes had spaces inside
    return trim(t);
}

static inline std::string toLowerStr(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (char c : s) out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return out;
}

}

Filter parseFilterSpec(const std::string& spec) {
    Filter f;
    if (spec.empty()) return f;

    std::istringstream ss(spec);
    std::string token;
    while (std::getline(ss, token, ';')) {
        token = trim(token);
        if (token.empty()) continue;
        // find op position
        const std::vector<std::string> ops = {">=", "<=", "!=", ">", "<", "="};
        size_t opPos = string::npos; string op;
        for (const auto& o : ops) {
            auto p = token.find(o);
            if (p != string::npos) { opPos = p; op = o; break; }
        }
        if (opPos == string::npos) continue;
        string key = trim(token.substr(0, opPos));
        string val = trim(token.substr(opPos + op.size()));
        if (key == "date") {
            int v = dateToInt(val);
            if (op == ">=") {
            f.addPredicate([v](const shared_ptr<Transaction>& t, const AppState&){ return dateToInt(cleanValue(t->bookingDate)) >= v; });
            } else if (op == "<=") {
                f.addPredicate([v](const shared_ptr<Transaction>& t, const AppState&){ return dateToInt(cleanValue(t->bookingDate)) <= v; });
            } else if (op == ">") {
                f.addPredicate([v](const shared_ptr<Transaction>& t, const AppState&){ return dateToInt(cleanValue(t->bookingDate)) > v; });
            } else if (op == "<") {
                f.addPredicate([v](const shared_ptr<Transaction>& t, const AppState&){ return dateToInt(cleanValue(t->bookingDate)) < v; });
            } else if (op == "=") {
                f.addPredicate([v](const shared_ptr<Transaction>& t, const AppState&){ return dateToInt(cleanValue(t->bookingDate)) == v; });
            }
        } else if (key == "amount") {
            double dv = 0.0; try { dv = stod(val); } catch(...) { dv = 0.0; }
            if (op == ">=") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount >= dv; });
            else if (op == "<=") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount <= dv; });
            else if (op == ">") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount > dv; });
            else if (op == "<") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount < dv; });
            else if (op == "=") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount == dv; });
            else if (op == "!=") f.addPredicate([dv](const shared_ptr<Transaction>& t, const AppState&){ return t->amount != dv; });
        } else if (key == "contract.type") {
            // support exact or OR list of types (pipe '|' or comma separated)
            string desired = val;
            if (desired.size() >= 2 && ((desired.front() == '"' && desired.back() == '"') || (desired.front() == '\'' && desired.back() == '\''))) desired = desired.substr(1, desired.size()-2);
            std::string s = toLowerStr(cleanValue(desired));
            std::vector<std::string> wanted;
            std::string cur;
            for (char ch : s) {
                if (ch == '|' || ch == ',') { auto t = trim(cur); if (!t.empty()) wanted.push_back(t); cur.clear(); }
                else cur.push_back(ch);
            }
            if (!cur.empty()) wanted.push_back(trim(cur));

            // trim whitespace on wanted parts
            for (auto &w : wanted) w = trim(w);

            // if no wanted parts, ignore
            if (wanted.empty()) continue;

            f.addPredicate([wanted](const shared_ptr<Transaction>& t, const AppState& st){
                if (!t) return false;
                // explicit support for 'unassigned' token: match transactions without contract
                bool anyUnassigned = false;
                for (const auto &w : wanted) if (w == "unassigned") anyUnassigned = true;
                if (anyUnassigned && t->contractId.empty()) return true;
                if (t->contractId.empty()) return false;
                // find the contract by id and compare its type exactly (case-insensitive)
                for (const auto& cptr : st.contracts) {
                    if (!cptr) continue;
                    if (toLowerStr(cleanValue(cptr->id)) == toLowerStr(cleanValue(t->contractId))) {
                        std::string ctype = toLowerStr(cleanValue(std::string(cptr->type)));
                        for (const auto &w : wanted) {
                            if (w == "unassigned") continue;
                            if (ctype == w) return true;
                        }
                        return false;
                    }
                }
                return false;
            });
        } else if (key == "propertyId") {
            string pid = val;
            if (pid.size() >= 2 && ((pid.front() == '"' && pid.back() == '"') || (pid.front() == '\'' && pid.back() == '\''))) pid = pid.substr(1, pid.size()-2);
            f.addPredicate([pid](const shared_ptr<Transaction>& t, const AppState&){ if (!t) return false; for (const auto& p : t->propertyIds) if (p == pid) return true; return false; });
        }
    }

    return f;
}
