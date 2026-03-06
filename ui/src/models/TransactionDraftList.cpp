#include "ui/models/TransactionDraftList.h"

#include <QVariant>

#include "ui/payload/UiPayloadKeys.h"

namespace ui {

TransactionDraftList::TransactionDraftList(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TransactionDraftList::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(drafts_.size());
}

QVariant TransactionDraftList::data(const QModelIndex& index, int role) const
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

QHash<int, QByteArray> TransactionDraftList::roleNames() const
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

void TransactionDraftList::setDrafts(std::vector<TransactionDraft> drafts)
{
    beginResetModel();
    drafts_ = std::move(drafts);
    endResetModel();
}

QVariantMap TransactionDraftList::get(int index) const
{
    QVariantMap m;
    if (index < 0 || index >= static_cast<int>(drafts_.size())) return m;
    const auto& t = drafts_[index];
    m[payload::keys::common::kName] = t.name;
    m[payload::keys::transaction::kBookingDate] = t.bookingDate;
    m[payload::keys::transaction::kValuta] = t.valuta;
    m[payload::keys::common::kAmount] = t.amount;
    m[payload::keys::common::kDescription] = t.description;
    m[payload::keys::transaction::kActorId] = t.actorId;
    m[payload::keys::transaction::kActorProposal] = t.actorProposal;
    m[payload::keys::common::kMetadata] = t.metadata;
    m[payload::keys::transaction::kProofImagePath] = t.proofImagePath;
    m[payload::keys::transaction::kAllocatable] = t.allocatable;
    m[payload::keys::common::kStatus] = t.status;
    m[payload::keys::transaction::kPropertyIds] = t.propertyIds;
    m[payload::keys::common::kType] = t.type;
    return m;
}

static bool validIndex(int idx, int size)
{
    return idx >= 0 && idx < size;
}

void TransactionDraftList::setActorId(int index, const QString& actorId)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].actorId == actorId) return;
    drafts_[index].actorId = actorId;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ActorIdRole });
}

void TransactionDraftList::setActorProposal(int index, const QString& actorProposal)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].actorProposal == actorProposal) return;
    drafts_[index].actorProposal = actorProposal;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ActorProposalRole });
}

void TransactionDraftList::setName(int index, const QString& name)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].name == name) return;
    drafts_[index].name = name;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { NameRole });
}

void TransactionDraftList::setBookingDate(int index, const QString& bookingDate)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].bookingDate == bookingDate) return;
    drafts_[index].bookingDate = bookingDate;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { BookingDateRole });
}

void TransactionDraftList::setValuta(int index, const QString& valuta)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].valuta == valuta) return;
    drafts_[index].valuta = valuta;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ValutaRole });
}

void TransactionDraftList::setAmount(int index, double amount)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].amount == amount) return;
    drafts_[index].amount = amount;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { AmountRole });
}

void TransactionDraftList::setDescription(int index, const QString& description)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].description == description) return;
    drafts_[index].description = description;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { DescriptionRole });
}

void TransactionDraftList::setStatus(int index, int status)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].status == status) return;
    drafts_[index].status = status;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { StatusRole });
}

void TransactionDraftList::setMetadata(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].metadata == v) return;
    drafts_[index].metadata = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { MetadataRole });
}

void TransactionDraftList::setProofImagePath(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].proofImagePath == v) return;
    drafts_[index].proofImagePath = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { ProofImagePathRole });
}

void TransactionDraftList::setAllocatable(int index, bool v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].allocatable == v) return;
    drafts_[index].allocatable = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { AllocatableRole });
}

void TransactionDraftList::setProperties(int index, const QStringList& ids)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].propertyIds == ids) return;
    drafts_[index].propertyIds = ids;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { PropertyIdsRole });
}

void TransactionDraftList::setType(int index, const QString& v)
{
    if (!validIndex(index, static_cast<int>(drafts_.size()))) return;
    if (drafts_[index].type == v) return;
    drafts_[index].type = v;
    const QModelIndex mi = this->index(index);
    emit dataChanged(mi, mi, { TypeRole });
}

}
