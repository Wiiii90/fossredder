#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Annual;
}

class IAnnualRepository {
public:
    virtual ~IAnnualRepository() = default;

    virtual void addAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;
    virtual std::vector<std::shared_ptr<core::domain::Annual>> getAnnuals() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::Annual>> getAnnualById(const std::string& id) const = 0;
    virtual void removeAnnual(const std::string& id) = 0;
    virtual void updateAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;
    virtual void upsertAnnual(const std::shared_ptr<core::domain::Annual>& annual) = 0;
    virtual void clearAnnuals() = 0;
};
