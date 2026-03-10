#include "ui/models/TransactionList.h"

#include <QVariant>
#include "ui/payload/PayloadKeys.h"

namespace ui {

namespace {

const QVector<int>& transactionRoles()
{
    static const QVector<int> roles{
        TransactionList::IdRole,
        TransactionList::StatementIdRole,
        TransactionList::NameRole,
        TransactionList::BookingDateRole,
        TransactionList::ValutaRole,
        TransactionList::AmountRole,
        TransactionList::DescriptionRole,
        TransactionList::StatusRole,
        TransactionList::ActorIdRole,
        TransactionList::ActorProposalRole,
        TransactionList::MetadataRole,
        TransactionList::ProofImagePathRole,
        TransactionList::TypeRole,
        TransactionList::AllocatableRole,
        TransactionList::PropertyIdsRole
    };
    return roles;
}

}

void TransactionList::rebuildIdIndex()
{
    idToRow_.clear();
    const auto& items = rows();
    idToRow_.reserve(static_cast<int>(items.size()));
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const auto& t = items[static_cast<size_t>(i)];
        if (!t) continue;
        idToRow_.insert(QString::fromStdString(t->id), i);
    }
}

QVector<int> TransactionList::allRoles() const
{
    return transactionRoles();
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

TransactionList::TransactionList(QObject* parent) : Base(parent) {}

QVariant TransactionList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto* entry = rowPtr(index.row());
    if (!entry || !*entry) return {};
    const auto& t = **entry;

    switch (role) {
    case IdRole: return QString::fromStdString(t.id);
    case StatementIdRole: return QString::fromStdString(t.statementId);
    case NameRole: return QString::fromStdString(t.name);
    case BookingDateRole: return QString::fromStdString(t.bookingDate);
    case ValutaRole: return QString::fromStdString(t.valuta);
    case AmountRole: return t.amount;
    case DescriptionRole: return QString::fromStdString(t.description);
    case StatusRole: return static_cast<int>(t.status);
    case ActorIdRole: return QString::fromStdString(t.actorId);
    case ActorProposalRole: return QString::fromStdString(t.actorProposal);
    case MetadataRole: return QString::fromStdString(t.metadata);
    case ProofImagePathRole: return QString::fromStdString(t.proofImagePath);
    case TypeRole: return contractTypeForTransaction(t);
    case AllocatableRole: return t.allocatable;
    case PropertyIdsRole: return toPropertyIdList(t.propertyIds);
    default: return {};
    }
}

QHash<int, QByteArray> TransactionList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = payload::keys::common::kId.toUtf8();
    roles[StatementIdRole] = payload::keys::statement::kStatementId.toUtf8();
    roles[NameRole] = payload::keys::common::kName.toUtf8();
    roles[BookingDateRole] = payload::keys::transaction::kBookingDate.toUtf8();
    roles[ValutaRole] = payload::keys::transaction::kValuta.toUtf8();
    roles[AmountRole] = payload::keys::common::kAmount.toUtf8();
    roles[DescriptionRole] = payload::keys::common::kDescription.toUtf8();
    roles[StatusRole] = payload::keys::common::kStatus.toUtf8();
    roles[ActorIdRole] = payload::keys::transaction::kActorId.toUtf8();
    roles[ActorProposalRole] = payload::keys::transaction::kActorProposal.toUtf8();
    roles[MetadataRole] = payload::keys::common::kMetadata.toUtf8();
    roles[ProofImagePathRole] = payload::keys::transaction::kProofImagePath.toUtf8();
    roles[TypeRole] = payload::keys::common::kType.toUtf8();
    roles[AllocatableRole] = payload::keys::transaction::kAllocatable.toUtf8();
    roles[PropertyIdsRole] = payload::keys::transaction::kPropertyIds.toUtf8();
    return roles;
}

void TransactionList::setTransactions(std::vector<std::shared_ptr<Transaction>> transactions)
{
    setRows(std::move(transactions));
    rebuildIdIndex();
}

void TransactionList::setContractTypes(QHash<QString, QString> contractTypes)
{
    contractTypeById_ = std::move(contractTypes);
    emitAllRowsChanged({TypeRole});
}

int TransactionList::findRowById(const QString& id) const
{
    if (id.isEmpty()) return -1;
    const auto it = idToRow_.find(id);
    return it == idToRow_.end() ? -1 : it.value();
}

void TransactionList::setTransactionAt(int row, std::shared_ptr<Transaction> tx)
{
    if (!hasRow(row)) return;
    replaceRow(row, std::move(tx));
    rebuildIdIndex();
    const QModelIndex modelIndex = index(row);
    emit dataChanged(modelIndex, modelIndex, allRoles());
}

void TransactionList::removeAt(int row)
{
    if (!hasRow(row)) return;
    removeRow(row);
    rebuildIdIndex();
}

QVariantMap TransactionList::get(int index) const
{
    QVariantMap m;
    const auto* entry = rowPtr(index);
    if (!entry || !*entry) return m;
    fillTransactionMap(m, **entry);
    return m;
}

}
