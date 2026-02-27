#pragma once

#include <QObject>
#include <QHash>
#include <QString>

class TransactionFilter;
class TransactionList;

class UiTransactionQueries {
public:
    QObject* transactionsForStatement(const QString& statementId, TransactionList* sourceModel, QObject* parent);
    QObject* transactionsForProperty(const QString& propertyId, TransactionList* sourceModel, QObject* parent);

    void clear();
    void removePropertyCache(const QString& propertyId);

private:
    QHash<QString, TransactionFilter*> byStatement_;
    QHash<QString, TransactionFilter*> byProperty_;
};
