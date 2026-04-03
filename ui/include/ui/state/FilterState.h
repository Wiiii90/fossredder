#pragma once

#include <QHash>
#include <QString>

class QObject;

namespace ui {

class TransactionList;
class TransactionFilter;

class FilterState {
public:
    explicit FilterState(QObject* owner = nullptr);

    TransactionFilter* statementTransactions(const QString& statementId, TransactionList& sourceModel);
    TransactionFilter* propertyTransactions(const QString& propertyId, TransactionList& sourceModel);

    void clear();
    void removeStatement(const QString& statementId);
    void removeProperty(const QString& propertyId);

private:
    QObject* owner_ = nullptr;
    QHash<QString, TransactionFilter*> statementFilters_;
    QHash<QString, TransactionFilter*> propertyFilters_;
};

}
