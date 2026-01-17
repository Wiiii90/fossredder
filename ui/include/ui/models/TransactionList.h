#pragma once

#include <QAbstractListModel>
#include <QString>

#include <memory>
#include <vector>

#include "core/models/Transaction.h"

class Contract;

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
        TypeRole,
        AllocatableRole,
        PropertyIdsRole
        // PaymentSchemeRole removed
    };

    explicit TransactionList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTransactions(std::vector<std::shared_ptr<Transaction>> transactions);
    void setContracts(std::vector<std::shared_ptr<Contract>> contracts);
    std::vector<std::shared_ptr<Transaction>> transactions() const;
    int findRowById(const QString& id) const;
    void setTransactionAt(int row, std::shared_ptr<Transaction> tx);
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE QVariantMap get(int index) const;

private:
    std::vector<std::shared_ptr<Transaction>> transactions_;
    std::vector<std::shared_ptr<Contract>> contracts_;
};
