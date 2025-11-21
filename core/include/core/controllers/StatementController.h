#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

class IStatementExtractionService;
class StatementData;
class Transaction;

class StatementController {
public:
    explicit StatementController(std::shared_ptr<IStatementExtractionService> extractionService = nullptr)
        : extractionService_(std::move(extractionService)) {}

    std::shared_ptr<StatementData> importStatement(const std::string& filePath);

    // Optional callback for notifying about extracted transactions
    void setTransactionsExtractedCallback(std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> cb) {
        onTransactionsExtracted_ = std::move(cb);
    }

private:
    std::shared_ptr<IStatementExtractionService> extractionService_;
    std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> onTransactionsExtracted_;
};
