#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>

class Transaction;

// Analysis represents a user-configurable analysis applied to a filtered set of
// transactions. The `configJson` field stores backend-opaque configuration data
// (JSON string) to avoid introducing a JSON dependency at this stage.
class Analysis {
public:
    Analysis() = default;

    std::string id;
    std::string name;
    std::string type; // e.g. "tab", "plot", "calc"

    // configuration serialized as JSON string
    std::string configJson;

    // serializable filter spec (JSON string)
    std::string filterSpec;

    // optional explicit adjustments: map from transaction id to adjusted amount
    // used by calc analyses to store per-transaction adjusted values
    std::unordered_map<std::string, double> adjustments;

    std::map<std::string, double> metrics;
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> artifacts;
    std::vector<std::shared_ptr<Transaction>> transactions;
    bool found = false;

    std::string createdAt;
    std::string updatedAt;

    int schemaVersion = 1;
};
