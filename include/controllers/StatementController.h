#pragma once

#include <QObject>
#include <memory>
#include <string>
#include <vector>
#include <utility>

class IStatementExtractionService;
class StatementData;
class Transaction;

class StatementController : public QObject {
    Q_OBJECT
public:
    // Construct with extraction service
    StatementController(std::shared_ptr<IStatementExtractionService> extractionService = nullptr, QObject* parent = nullptr)
        : QObject(parent), extractionService_(std::move(extractionService)) {}

    std::shared_ptr<StatementData> importStatement(const std::string& filePath);

signals:
    void transactionsExtracted(const std::vector<std::shared_ptr<Transaction>>& transactions);

private:
    std::shared_ptr<IStatementExtractionService> extractionService_;
};
