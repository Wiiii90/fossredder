#pragma once

#include <QObject>
#include <memory>
#include <QString>
#include <QList>
#include <QVariant>
#include <vector>

class StatementController;
class Transaction;

class QTStatementController : public QObject {
    Q_OBJECT
public:
    explicit QTStatementController(std::shared_ptr<StatementController> coreController, QObject* parent = nullptr);

public slots:
    void importStatement(const QString& path);

signals:
    void transactionsExtracted(const QList<QVariant>& transactions);

private:
    std::shared_ptr<StatementController> coreController_;
};
