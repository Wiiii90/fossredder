#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class ImportLog;
}

class IImportLogRepository {
public:
    virtual ~IImportLogRepository() = default;

    virtual void addImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;
    virtual std::vector<std::shared_ptr<core::domain::ImportLog>> getImportLogs() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::ImportLog>> getImportLogById(const std::string& id) const = 0;
    virtual void removeImportLog(const std::string& id) = 0;
    virtual void updateImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;
    virtual void upsertImportLog(const std::shared_ptr<core::domain::ImportLog>& log) = 0;
    virtual void clearImportLogs() = 0;
};
