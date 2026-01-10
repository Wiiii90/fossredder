#include "ui/models/TransactionList.h"

#include <QVariant>

TransactionList::TransactionList(QObject* parent) : QAbstractListModel(parent) {}

int TransactionList::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(transactions_.size());
}

QVariant TransactionList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(transactions_.size())) return {};
    const auto& t = transactions_[row];
    if (!t) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(t->id);
    case StatementIdRole: return QString::fromStdString(t->statementId);
    case NameRole: return QString::fromStdString(t->name);
    case BookingDateRole: return QString::fromStdString(t->bookingDate);
    case ValutaRole: return QString::fromStdString(t->valuta);
    case AmountRole: return t->amount;
    case DescriptionRole: return QString::fromStdString(t->description);
    case StatusRole: return static_cast<int>(t->status);
    case ActorIdRole: return QString::fromStdString(t->actorId);
    case ActorProposalRole: return QString::fromStdString(t->actorProposal);
    case MetadataRole: return QString::fromStdString(t->metadata);
    case ProofImagePathRole: return QString::fromStdString(t->proofImagePath);
    case AllocatableRole: return t->allocatable;
    case PropertyIdsRole: {
        QVariantList out;
        for (const auto& pid : t->propertyIds) out.push_back(QString::fromStdString(pid));
        return out;
    }
    default: return {};
    }
}

QHash<int, QByteArray> TransactionList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[StatementIdRole] = "statementId";
    roles[NameRole] = "name";
    roles[BookingDateRole] = "bookingDate";
    roles[ValutaRole] = "valuta";
    roles[AmountRole] = "amount";
    roles[DescriptionRole] = "description";
    roles[StatusRole] = "status";
    roles[ActorIdRole] = "actorId";
    roles[ActorProposalRole] = "actorProposal";
    roles[MetadataRole] = "metadata";
    roles[ProofImagePathRole] = "proofImagePath";
    roles[AllocatableRole] = "allocatable";
    roles[PropertyIdsRole] = "propertyIds";
    return roles;
}

void TransactionList::setTransactions(std::vector<std::shared_ptr<Transaction>> transactions)
{
    beginResetModel();
    transactions_ = std::move(transactions);
    endResetModel();
}

std::vector<std::shared_ptr<Transaction>> TransactionList::transactions() const
{
    return transactions_;
}
