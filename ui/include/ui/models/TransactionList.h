#pragma once

#include <QAbstractListModel>
#include <QString>

#include <memory>
#include <vector>

#include "core/models/Transaction.h"

class TransactionList : public QAbstractListModel {
    Q_OBJECT
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
        ActorProposalRole,
        MetadataRole,
        ProofImagePathRole,
        AllocatableRole
        // PaymentSchemeRole removed
    };

    explicit TransactionList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTransactions(std::vector<std::shared_ptr<Transaction>> transactions);
    std::vector<std::shared_ptr<Transaction>> transactions() const;

private:
    std::vector<std::shared_ptr<Transaction>> transactions_;
};
