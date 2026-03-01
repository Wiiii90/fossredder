#pragma once

#include <QHash>
#include <QString>

class QObject;

namespace ui {

class TransactionList;
class TransactionFilter;

class FilterState {
public:
    QObject* statementTransactions(const QString& statementId, TransactionList& sourceModel, QObject* parent);
    QObject* propertyTransactions(const QString& propertyId, TransactionList& sourceModel, QObject* parent);

    void clear();
    void removeStatement(const QString& statementId);
    void removeProperty(const QString& propertyId);

private:
    QHash<QString, TransactionFilter*> statementFilters_;
    QHash<QString, TransactionFilter*> propertyFilters_;
};

}
