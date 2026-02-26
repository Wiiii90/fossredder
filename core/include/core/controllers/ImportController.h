#pragma once

#include <memory>
#include <string>
#include <functional>
#include <atomic>

namespace core { namespace jobs { class Scheduler; class SlotLimiter; } }

class Statement;
class StatementController;

class ImportController {
public:
    enum class ImportType {
        Statement
    };

    explicit ImportController(std::shared_ptr<StatementController> statementController);

    std::shared_ptr<Statement> import(ImportType type,
                                      const std::string& filePath,
                                      const std::string& runRoot = {},
                                      const std::string& runIdPrefix = {},
                                      std::function<void(double, const std::string&)> progressCallback = {},
                                      std::shared_ptr<std::atomic<bool>> cancelFlag = nullptr,
                                      core::jobs::Scheduler* scheduler = nullptr,
                                      core::jobs::SlotLimiter* ocrLimiter = nullptr,
                                      std::string jobId = {});

private:
    std::shared_ptr<StatementController> statementController_;
};
