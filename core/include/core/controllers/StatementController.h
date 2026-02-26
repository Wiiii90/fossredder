#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <atomic>

class IImportStatement;
class Statement;
class Transaction;

namespace core { namespace jobs { class Scheduler; class SlotLimiter; } }

class StatementController {
public:
    explicit StatementController(std::shared_ptr<IImportStatement> importService = nullptr)
        : importService_(std::move(importService)) {}

    std::shared_ptr<Statement> importStatement(const std::string& filePath,
                                               const std::string& runRoot = {},
                                               const std::string& runIdPrefix = {},
                                               std::function<void(double, const std::string&)> progressCallback = {},
                                               std::shared_ptr<std::atomic<bool>> cancelFlag = nullptr,
                                               core::jobs::Scheduler* scheduler = nullptr,
                                               core::jobs::SlotLimiter* ocrLimiter = nullptr,
                                               std::string jobId = {});

    // Optional callback for notifying about extracted transactions
    void setTransactionsExtractedCallback(std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> cb) {
        onTransactionsExtracted_ = std::move(cb);
    }

    // Note: persistence is handled by higher layers (UI/AppStateController). StatementController only reports parsed data.
    // void setAppStateController(AppStateController* core) { appStateController_ = core; }  // removed

private:
    std::shared_ptr<IImportStatement> importService_;
    std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> onTransactionsExtracted_;
    // AppStateController* appStateController_ = nullptr;  // removed
};
