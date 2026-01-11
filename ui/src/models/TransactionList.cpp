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
    case TypeRole: return QString::fromStdString(t->type);
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
    roles[TypeRole] = "type";
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

int TransactionList::findRowById(const QString& id) const
{
    if (id.isEmpty()) return -1;
    for (int i = 0; i < static_cast<int>(transactions_.size()); ++i) {
        const auto& t = transactions_[static_cast<size_t>(i)];
        if (!t) continue;
        if (QString::fromStdString(t->id) == id) return i;
    }
    return -1;
}

void TransactionList::setTransactionAt(int row, std::shared_ptr<Transaction> tx)
{
    if (row < 0 || row >= static_cast<int>(transactions_.size())) return;
    transactions_[static_cast<size_t>(row)] = std::move(tx);
    const QModelIndex mi = index(row);
    // build QVector<int> of roles
    QVector<int> rolesVec;
    const auto roleKeys = roleNames().keys();
    rolesVec.reserve(roleKeys.size());
    for (int k : roleKeys) rolesVec.append(k);
    emit dataChanged(mi, mi, rolesVec);
}

void TransactionList::removeAt(int row)
{
    if (row < 0 || row >= static_cast<int>(transactions_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    transactions_.erase(transactions_.begin() + row);
    endRemoveRows();
}
