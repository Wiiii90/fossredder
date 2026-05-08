/**
 * @file ui/src/models/TransactionDraftList.cpp
 * @brief Implementation of the UI TransactionDraftList component.
 */

#include "ui/models/TransactionDraftList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"
#include "ui/models/ImportSuggestion.h"
#include "ui/payload/PayloadMapper.h"

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
    case ActorTextRole: return t->actorText;
    case ActorIdRole: return t->actorId;
    case ActorSelectedRole: return t->actorSelected;
    case ContractIdRole: return t->contractId;
    case ContractSelectedRole: return t->contractSelected;
    case MetadataRole: return t->metadata;
    case ProofImageDataRole: return t->proofImageData;
    case ActorSuggestionsRole: return ui::toVariantMap(t->suggestions.actor);
    case PropertySuggestionsRole: return ui::toVariantMap(t->suggestions.property);
    case ContractSuggestionsRole: return ui::toVariantMap(t->suggestions.contract);
    case AllocatableRole: return t->allocatable;
    case AllocatableSelectedRole: return t->allocatableSelected;
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
    roles[ActorTextRole] = payload::keys::draft::kActorText.toUtf8();
    roles[ActorIdRole] = payload::keys::transaction::kActorId.toUtf8();
    roles[ActorSelectedRole] = payload::keys::draft::kActorSelected.toUtf8();
    roles[ContractIdRole] = payload::keys::transaction::kContractId.toUtf8();
    roles[ContractSelectedRole] = payload::keys::draft::kContractSelected.toUtf8();
    roles[MetadataRole] = payload::keys::common::kMetadata.toUtf8();
    roles[ProofImageDataRole] = payload::keys::transaction::kProofImageData.toUtf8();
    roles[ActorSuggestionsRole] = payload::keys::draft::kActorSuggestions.toUtf8();
    roles[PropertySuggestionsRole] = payload::keys::draft::kPropertySuggestions.toUtf8();
    roles[ContractSuggestionsRole] = payload::keys::draft::kContractSuggestions.toUtf8();
    roles[AllocatableRole] = payload::keys::transaction::kAllocatable.toUtf8();
    roles[AllocatableSelectedRole] = payload::keys::draft::kAllocatableSelected.toUtf8();
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
    m[payload::keys::draft::kActorText] = t->actorText;
    m[payload::keys::transaction::kActorId] = t->actorId;
    m[payload::keys::draft::kActorSelected] = t->actorSelected;
    m[payload::keys::transaction::kContractId] = t->contractId;
    m[payload::keys::draft::kContractSelected] = t->contractSelected;
    m[payload::keys::common::kMetadata] = t->metadata;
    m[payload::keys::transaction::kProofImageData] = t->proofImageData;
    m[payload::keys::draft::kActorSuggestions] = ui::toVariantMap(t->suggestions.actor);
    m[payload::keys::draft::kPropertySuggestions] = ui::toVariantMap(t->suggestions.property);
    m[payload::keys::draft::kContractSuggestions] = ui::toVariantMap(t->suggestions.contract);
    m[payload::keys::transaction::kAllocatable] = t->allocatable;
    m[payload::keys::draft::kAllocatableSelected] = t->allocatableSelected;
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

void TransactionDraftList::setActorText(int index, const QString& actorText)
{
    updateField(index, actorText, ActorTextRole, [](TransactionDraft& draft) -> QString& {
        return draft.actorText;
    });
}

void TransactionDraftList::setActorSelected(int index, bool selected)
{
    updateField(index, selected, ActorSelectedRole, [](TransactionDraft& draft) -> bool& {
        return draft.actorSelected;
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

void TransactionDraftList::setContractId(int index, const QString& contractId)
{
    updateField(index, contractId, ContractIdRole, [](TransactionDraft& draft) -> QString& {
        return draft.contractId;
    });
}

void TransactionDraftList::setContractSelected(int index, bool selected)
{
    updateField(index, selected, ContractSelectedRole, [](TransactionDraft& draft) -> bool& {
        return draft.contractSelected;
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

void TransactionDraftList::setProofImageData(int index, const QString& v)
{
    updateField(index, v, ProofImageDataRole, [](TransactionDraft& draft) -> QString& {
        return draft.proofImageData;
    });
}

void TransactionDraftList::setAllocatable(int index, bool v)
{
    updateField(index, v, AllocatableRole, [](TransactionDraft& draft) -> bool& {
        return draft.allocatable;
    });
}

void TransactionDraftList::setAllocatableSelected(int index, bool v)
{
    updateField(index, v, AllocatableSelectedRole, [](TransactionDraft& draft) -> bool& {
        return draft.allocatableSelected;
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

void TransactionDraftList::removeAt(int index)
{
    removeRow(index);
}

}

