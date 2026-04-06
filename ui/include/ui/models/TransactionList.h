/**
 * @file ui/include/ui/models/TransactionList.h
 * @brief Declarations for the UI TransactionList component.
 */

#pragma once

#include <QHash>
#include <QString>
#include <QVariant>

#include "core/models/Transaction.h"
#include "ui/models/RowListModel.h"

namespace ui {

class TransactionList : public models::RowListModel<std::shared_ptr<core::domain::Transaction>> {
    Q_OBJECT
    using Base = models::RowListModel<std::shared_ptr<core::domain::Transaction>>;
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        StatementIdRole,
        NameRole,
        BookingDateRole,
        ValutaRole,
        AmountRole,
        DescriptionRole,
        StatusRole,
        ActorIdRole,
        MetadataRole,
        ProofImagePathRole,
        TypeRole,
        AllocatableRole,
        PropertyIdsRole
        // PaymentSchemeRole removed
    };

    explicit TransactionList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTransactions(std::vector<std::shared_ptr<core::domain::Transaction>> transactions);
    void setContractTypes(QHash<QString, QString> contractTypes, bool notify = true);
    const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions() const { return rows(); }
    int findRowById(const QString& id) const;
    void setTransactionAt(int row, std::shared_ptr<core::domain::Transaction> tx);
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE QVariantMap get(int index) const;

private:
    void rebuildIdIndex();
    QString contractTypeForTransaction(const core::domain::Transaction& transaction) const;
    void fillTransactionMap(QVariantMap& map, const core::domain::Transaction& transaction) const;
    QVector<int> allRoles() const;

    QHash<QString, int> idToRow_;
    QHash<QString, QString> contractTypeById_;
};

}
