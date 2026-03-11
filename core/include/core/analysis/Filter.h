#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace core::domain {
struct AppState;
class Transaction;
}

namespace core::analysis {

// Simple filter object produced by parseFilterSpec. Predicates are ANDed.
class Filter {
public:
    using Pred = std::function<bool(const std::shared_ptr<core::domain::Transaction>&, const core::domain::AppState&)>;

    void addPredicate(Pred p) { preds_.push_back(std::move(p)); }

    bool matches(const std::shared_ptr<core::domain::Transaction>& t, const core::domain::AppState& state) const {
        for (const auto& p : preds_) if (!p(t, state)) return false;
        return true;
    }

    bool empty() const { return preds_.empty(); }

private:
    std::vector<Pred> preds_;
};

// Parse a compact filter specification into a Filter object.
// Syntax (simple): clauses separated by ';' where each clause is
// <key><op><value> with op in {=,!=,>=,<=,>,<}
// Supported keys: date (YYYY-MM-DD), amount (numeric), contract.type (string), propertyId (string contains)
Filter parseFilterSpec(const std::string& spec);

}
