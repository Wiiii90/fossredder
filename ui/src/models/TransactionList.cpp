#include "ui/models/TransactionList.h"

#include <QVariant>
#include "core/models/Contract.h"
#include "ui/payload/PayloadKeys.h"

namespace ui {

void TransactionList::rebuildIdIndex()
{
    idToRow_.clear();
    idToRow_.reserve(static_cast<int>(transactions_.size()));
    for (int i = 0; i < static_cast<int>(transactions_.size()); ++i) {
        const auto& t = transactions_[static_cast<size_t>(i)];
        if (!t) continue;
        idToRow_.insert(QString::fromStdString(t->id), i);
    }
}

void TransactionList::rebuildContractTypeIndex()
{
    contractTypeById_.clear();
    contractTypeById_.reserve(static_cast<int>(contracts_.size()));
    for (const auto& contract : contracts_) {
        if (!contract) continue;
        contractTypeById_.insert(QString::fromStdString(contract->id), QString::fromStdString(contract->type));
    }
}

QString TransactionList::contractTypeForTransaction(const Transaction& transaction) const
{
    if (transaction.contractId.empty()) return {};
    const QString contractId = QString::fromStdString(transaction.contractId);
    const auto it = contractTypeById_.find(contractId);
    return it == contractTypeById_.end() ? QString() : it.value();
}

QVariantList TransactionList::toPropertyIdList(const std::vector<std::string>& propertyIds)
{
    QVariantList out;
    out.reserve(static_cast<int>(propertyIds.size()));
    for (const auto& propertyId : propertyIds) {
        out.push_back(QString::fromStdString(propertyId));
    }
    return out;
}

void TransactionList::fillTransactionMap(QVariantMap& map, const Transaction& transaction) const
{
    map[payload::keys::common::kId] = QString::fromStdString(transaction.id);
    map[payload::keys::common::kName] = QString::fromStdString(transaction.name);
    map[payload::keys::transaction::kBookingDate] = QString::fromStdString(transaction.bookingDate);
    map[payload::keys::transaction::kValuta] = QString::fromStdString(transaction.valuta);
    map[payload::keys::common::kAmount] = transaction.amount;
    map[payload::keys::common::kDescription] = QString::fromStdString(transaction.description);
    map[payload::keys::common::kStatus] = static_cast<int>(transaction.status);
    map[payload::keys::transaction::kActorId] = QString::fromStdString(transaction.actorId);
    map[payload::keys::transaction::kActorProposal] = QString::fromStdString(transaction.actorProposal);
    map[payload::keys::common::kMetadata] = QString::fromStdString(transaction.metadata);
    map[payload::keys::transaction::kProofImagePath] = QString::fromStdString(transaction.proofImagePath);
    map[payload::keys::common::kType] = contractTypeForTransaction(transaction);
    map[payload::keys::transaction::kAllocatable] = transaction.allocatable;
    map[payload::keys::transaction::kPropertyIds] = toPropertyIdList(transaction.propertyIds);
}

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
    case TypeRole: return contractTypeForTransaction(*t);
    case AllocatableRole: return t->allocatable;
    case PropertyIdsRole: return toPropertyIdList(t->propertyIds);
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
    rebuildIdIndex();
    endResetModel();
}

void TransactionList::setContracts(std::vector<std::shared_ptr<::Contract>> contracts)
{
    contracts_ = std::move(contracts);
    rebuildContractTypeIndex();
    const int rows = rowCount();
    if (rows == 0) return;
    emit dataChanged(index(0), index(rows - 1), { TypeRole });
}

const std::vector<std::shared_ptr<Transaction>>& TransactionList::transactions() const
{
    return transactions_;
}

int TransactionList::findRowById(const QString& id) const
{
    if (id.isEmpty()) return -1;
    const auto it = idToRow_.find(id);
    return it == idToRow_.end() ? -1 : it.value();
}

void TransactionList::setTransactionAt(int row, std::shared_ptr<Transaction> tx)
{
    if (row < 0 || row >= static_cast<int>(transactions_.size())) return;
    transactions_[static_cast<size_t>(row)] = std::move(tx);
    rebuildIdIndex();
    const QModelIndex mi = index(row);
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
    rebuildIdIndex();
    endRemoveRows();
}

QVariantMap TransactionList::get(int index) const
{
    QVariantMap m;
    if (index < 0 || index >= static_cast<int>(transactions_.size())) return m;
    const auto& t = transactions_.at(static_cast<size_t>(index));
    if (!t) return m;
    fillTransactionMap(m, *t);
    return m;
}

}
