#pragma once

#include <optional>
#include <string>

namespace core::storage {

class IRegistry {
public:
    virtual ~IRegistry() = default;
    virtual std::optional<std::string> getLatest() const = 0;
    virtual void setLatest(const std::string& path) = 0;
};

}
