#include "ui/models/TransactionDraftListModel.h"

#include <QVariant>

TransactionDraftListModel::TransactionDraftListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TransactionDraftListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(drafts_.size());
}

QVariant TransactionDraftListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(drafts_.size())) return {};
    const auto& t = drafts_[row];

    switch (role) {
    case NameRole: return t.name;
    case BookingDateRole: return t.bookingDate;
    case ValutaRole: return t.valuta;
    case AmountRole: return t.amount;
    case DescriptionRole: return t.description;
    case ActorIdRole: return t.actorId;
    case ActorProposalRole: return t.actorProposal;
    case MetadataRole: return t.metadata;
    case ProofImagePathRole: return t.proofImagePath;
    case AllocatableRole: return t.allocatable;
    case StatusRole: return t.status;
    case PropertyIdsRole: return QVariant::fromValue(t.propertyIds);
    case TypeRole: return t.type;
    default: return {};
    }
}

QHash<int, QByteArray> TransactionDraftListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[BookingDateRole] = "bookingDate";
    roles[ValutaRole] = "valuta";
    roles[AmountRole] = "amount";
    roles[DescriptionRole] = "description";
    roles[ActorIdRole] = "actorId";
    roles[ActorProposalRole] = "actorProposal";
    roles[MetadataRole] = "metadata";
    roles[ProofImagePathRole] = "proofImagePath";
    roles[AllocatableRole] = "allocatable";
    roles[StatusRole] = "status";
    roles[PropertyIdsRole] = "propertyIds";
    roles[TypeRole] = "type";
    return roles;
}

void TransactionDraftListModel::setDrafts(std::vector<TransactionDraft> drafts)
{
    beginResetModel();
    drafts_ = std::move(drafts);
    endResetModel();
}

QVariantMap TransactionDraftListModel::get(int index) const
{
    QVariantMap m;
    if (index < 0 || index >= static_cast<int>(drafts_.size())) return m;
    const auto& t = drafts_[index];
    m["name"] = t.name;
    m["bookingDate"] = t.bookingDate;
    m["valuta"] = t.valuta;
    m["amount"] = t.amount;
    m["description"] = t.description;
    m["actorId"] = t.actorId;
    m["actorProposal"] = t.actorProposal;
    m["metadata"] = t.metadata;
    m["proofImagePath"] = t.proofImagePath;
    m["allocatable"] = t.allocatable;
    m["status"] = t.status;
    m["propertyIds"] = t.propertyIds;
    m["type"] = t.type;
    return m;
}

static bool validIndex(int idx, int size) {
    return idx >= 0 && idx < size;
}

void TransactionDraftListModel::setActorId(int index, const QString& actorId)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].actorId == actorId) return;
    drafts_[index].actorId = actorId;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ActorIdRole });
}

void TransactionDraftListModel::setActorProposal(int index, const QString& actorProposal)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].actorProposal == actorProposal) return;
    drafts_[index].actorProposal = actorProposal;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ActorProposalRole });
}

void TransactionDraftListModel::setName(int index, const QString& name)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].name == name) return;
    drafts_[index].name = name;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { NameRole });
}

void TransactionDraftListModel::setBookingDate(int index, const QString& bookingDate)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].bookingDate == bookingDate) return;
    drafts_[index].bookingDate = bookingDate;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { BookingDateRole });
}

void TransactionDraftListModel::setValuta(int index, const QString& valuta)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].valuta == valuta) return;
    drafts_[index].valuta = valuta;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ValutaRole });
}

void TransactionDraftListModel::setAmount(int index, double amount)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].amount == amount) return;
    drafts_[index].amount = amount;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { AmountRole });
}

void TransactionDraftListModel::setDescription(int index, const QString& description)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].description == description) return;
    drafts_[index].description = description;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { DescriptionRole });
}

void TransactionDraftListModel::setStatus(int index, int status)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].status == status) return;
    drafts_[index].status = status;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { StatusRole });
}

void TransactionDraftListModel::setMetadata(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].metadata == v) return;
    drafts_[index].metadata = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { MetadataRole });
}

void TransactionDraftListModel::setProofImagePath(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].proofImagePath == v) return;
    drafts_[index].proofImagePath = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ProofImagePathRole });
}

void TransactionDraftListModel::setAllocatable(int index, bool v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].allocatable == v) return;
    drafts_[index].allocatable = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { AllocatableRole });
}

void TransactionDraftListModel::setProperties(int index, const QStringList& ids)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].propertyIds == ids) return;
    drafts_[index].propertyIds = ids;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { PropertyIdsRole });
}

void TransactionDraftListModel::setType(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].type == v) return;
    drafts_[index].type = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { TypeRole });
}
