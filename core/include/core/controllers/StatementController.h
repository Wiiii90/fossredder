#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

class IImportStatement;
class Statement;
class Transaction;

class StatementController {
public:
    explicit StatementController(std::shared_ptr<IImportStatement> importService = nullptr)
        : importService_(std::move(importService)) {}

    std::shared_ptr<Statement> importStatement(const std::string& filePath, const std::string& runRoot = {}, const std::string& runIdPrefix = {});

    // Optional callback for notifying about extracted transactions
    void setTransactionsExtractedCallback(std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> cb) {
        onTransactionsExtracted_ = std::move(cb);
    }

private:
    std::shared_ptr<IImportStatement> importService_;
    std::function<void(const std::vector<std::shared_ptr<Transaction>>&)> onTransactionsExtracted_;
};
