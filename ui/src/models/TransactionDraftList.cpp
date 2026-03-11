#include "ui/models/TransactionDraftList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"

namespace ui {

TransactionDraftList::TransactionDraftList(QObject* parent)
    : Base(parent)
{
}

QVariant TransactionDraftList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto* t = rowPtr(index.row());
    if (!t) return {};

    switch (role) {
    case NameRole: return t->name;
    case BookingDateRole: return t->bookingDate;
    case ValutaRole: return t->valuta;
    case AmountRole: return t->amount;
    case DescriptionRole: return t->description;
    case ActorIdRole: return t->actorId;
    case MetadataRole: return t->metadata;
    case ProofImagePathRole: return t->proofImagePath;
    case AllocatableRole: return t->allocatable;
    case StatusRole: return t->status;
    case PropertyIdsRole: return QVariant::fromValue(t->propertyIds);
    case TypeRole: return t->type;
    default: return {};
    }
}

QHash<int, QByteArray> TransactionDraftList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = payload::keys::common::kName.toUtf8();
    roles[BookingDateRole] = payload::keys::transaction::kBookingDate.toUtf8();
    roles[ValutaRole] = payload::keys::transaction::kValuta.toUtf8();
    roles[AmountRole] = payload::keys::common::kAmount.toUtf8();
    roles[DescriptionRole] = payload::keys::common::kDescription.toUtf8();
    roles[ActorIdRole] = payload::keys::transaction::kActorId.toUtf8();
    roles[MetadataRole] = payload::keys::common::kMetadata.toUtf8();
    roles[ProofImagePathRole] = payload::keys::transaction::kProofImagePath.toUtf8();
    roles[AllocatableRole] = payload::keys::transaction::kAllocatable.toUtf8();
    roles[StatusRole] = payload::keys::common::kStatus.toUtf8();
    roles[PropertyIdsRole] = payload::keys::transaction::kPropertyIds.toUtf8();
    roles[TypeRole] = payload::keys::common::kType.toUtf8();
    return roles;
}

void TransactionDraftList::setDrafts(std::vector<TransactionDraft> drafts)
{
    setRows(std::move(drafts));
}

QVariantMap TransactionDraftList::get(int index) const
{
    QVariantMap m;
    const auto* t = rowPtr(index);
    if (!t) return m;
    m[payload::keys::common::kName] = t->name;
    m[payload::keys::transaction::kBookingDate] = t->bookingDate;
    m[payload::keys::transaction::kValuta] = t->valuta;
    m[payload::keys::common::kAmount] = t->amount;
    m[payload::keys::common::kDescription] = t->description;
    m[payload::keys::transaction::kActorId] = t->actorId;
    m[payload::keys::common::kMetadata] = t->metadata;
    m[payload::keys::transaction::kProofImagePath] = t->proofImagePath;
    m[payload::keys::transaction::kAllocatable] = t->allocatable;
    m[payload::keys::common::kStatus] = t->status;
    m[payload::keys::transaction::kPropertyIds] = t->propertyIds;
    m[payload::keys::common::kType] = t->type;
    return m;
}

void TransactionDraftList::setActorId(int index, const QString& actorId)
{
    updateField(index, actorId, ActorIdRole, [](TransactionDraft& draft) -> QString& {
        return draft.actorId;
    });
}

void TransactionDraftList::setName(int index, const QString& name)
{
    updateField(index, name, NameRole, [](TransactionDraft& draft) -> QString& {
        return draft.name;
    });
}

void TransactionDraftList::setBookingDate(int index, const QString& bookingDate)
{
    updateField(index, bookingDate, BookingDateRole, [](TransactionDraft& draft) -> QString& {
        return draft.bookingDate;
    });
}

void TransactionDraftList::setValuta(int index, const QString& valuta)
{
    updateField(index, valuta, ValutaRole, [](TransactionDraft& draft) -> QString& {
        return draft.valuta;
    });
}

void TransactionDraftList::setAmount(int index, double amount)
{
    updateField(index, amount, AmountRole, [](TransactionDraft& draft) -> double& {
        return draft.amount;
    });
}

void TransactionDraftList::setDescription(int index, const QString& description)
{
    updateField(index, description, DescriptionRole, [](TransactionDraft& draft) -> QString& {
        return draft.description;
    });
}

void TransactionDraftList::setStatus(int index, int status)
{
    updateField(index, status, StatusRole, [](TransactionDraft& draft) -> int& {
        return draft.status;
    });
}

void TransactionDraftList::setMetadata(int index, const QString& v)
{
    updateField(index, v, MetadataRole, [](TransactionDraft& draft) -> QString& {
        return draft.metadata;
    });
}

void TransactionDraftList::setProofImagePath(int index, const QString& v)
{
    updateField(index, v, ProofImagePathRole, [](TransactionDraft& draft) -> QString& {
        return draft.proofImagePath;
    });
}

void TransactionDraftList::setAllocatable(int index, bool v)
{
    updateField(index, v, AllocatableRole, [](TransactionDraft& draft) -> bool& {
        return draft.allocatable;
    });
}

void TransactionDraftList::setProperties(int index, const QStringList& ids)
{
    updateField(index, ids, PropertyIdsRole, [](TransactionDraft& draft) -> QStringList& {
        return draft.propertyIds;
    });
}

void TransactionDraftList::setType(int index, const QString& v)
{
    updateField(index, v, TypeRole, [](TransactionDraft& draft) -> QString& {
        return draft.type;
    });
}

}
