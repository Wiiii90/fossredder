#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class ExportLog;
}

class IExportLogRepository {
public:
    virtual ~IExportLogRepository() = default;

    virtual void addExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;
    virtual std::vector<std::shared_ptr<core::domain::ExportLog>> getExportLogs() const = 0;
    virtual std::optional<std::shared_ptr<core::domain::ExportLog>> getExportLogById(const std::string& id) const = 0;
    virtual void removeExportLog(const std::string& id) = 0;
    virtual void updateExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;
    virtual void upsertExportLog(const std::shared_ptr<core::domain::ExportLog>& log) = 0;
    virtual void clearExportLogs() = 0;
};
