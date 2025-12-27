#pragma once

#include <QObject>
#include <memory>
#include <QString>
#include <QList>
#include <QVariant>

class StatementController;

class UiStatementController : public QObject {
    Q_OBJECT
public:
    explicit UiStatementController(std::shared_ptr<StatementController> coreController, QObject* parent = nullptr);

public slots:
    void importStatement(const QString& path);

signals:
    void transactionsExtracted(const QList<QVariant>& transactions);

private:
    std::shared_ptr<StatementController> coreController_;
};
