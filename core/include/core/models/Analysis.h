#pragma once

#include <string>
#include <map>

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

    std::string createdAt;
    std::string updatedAt;

    int schemaVersion = 1;
};
