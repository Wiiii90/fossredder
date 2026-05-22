/**
 * @file ui/include/ui/models/TransactionDraftList.h
 * @brief Declarations for the UI TransactionDraftList component.
 */

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "ui/viewmodels/base/RowListModel.h"
#include "ui/viewmodels/import/TransactionDraftViewModel.h"

namespace ui {

class TransactionDraftList : public models::RowListModel<TransactionDraft> {
    Q_OBJECT
    QML_NAMED_ELEMENT(TransactionDraftList)
    QML_UNCREATABLE("TransactionDraftList is exposed by the application context")
    using Base = models::RowListModel<TransactionDraft>;
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        BookingDateRole,
        ValutaRole,
        AmountRole,
        ActorTextRole,
        ActorIdRole,
        ActorSelectedRole,
        ContractIdRole,
        ContractSelectedRole,
        MetadataRole,
        ProofImageDataRole,
        ActorSuggestionsRole,
        PropertySuggestionsRole,
        ContractSuggestionsRole,
        AllocatableRole,
        AllocatableSelectedRole,
        StatusRole,
        PropertyIdsRole,
        TypeRole
    };

    explicit TransactionDraftList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setDrafts(std::vector<TransactionDraft> drafts);
    const std::vector<TransactionDraft>& drafts() const noexcept { return rows(); }

    Q_INVOKABLE QVariantMap get(int index) const;

    Q_INVOKABLE void setActorId(int index, const QString& actorId);
    Q_INVOKABLE void setActorText(int index, const QString& actorText);
    Q_INVOKABLE void setActorSelected(int index, bool selected);

    Q_INVOKABLE void setName(int index, const QString& name);
    Q_INVOKABLE void setBookingDate(int index, const QString& bookingDate);
    Q_INVOKABLE void setValuta(int index, const QString& valuta);
    Q_INVOKABLE void setAmount(int index, double amount);
    Q_INVOKABLE void setContractId(int index, const QString& contractId);
    Q_INVOKABLE void setContractSelected(int index, bool selected);

    Q_INVOKABLE void setMetadata(int index, const QString& v);
    Q_INVOKABLE void setProofImageData(int index, const QString& v);

    Q_INVOKABLE void setStatus(int index, int status);
    Q_INVOKABLE void setAllocatable(int index, bool v);
    Q_INVOKABLE void setAllocatableSelected(int index, bool v);

    Q_INVOKABLE void setProperties(int index, const QStringList& ids);
    Q_INVOKABLE void setType(int index, const QString& v);
    Q_INVOKABLE void removeAt(int index);

private:
    template <typename TValue, typename Accessor>
    void updateField(int index, TValue value, int role, Accessor&& accessor)
    {
        auto* draft = rowPtr(index);
        if (!draft) return;

        auto& field = accessor(*draft);
        if (field == value) return;

        field = std::move(value);
        emitRowChanged(index, {role});
    }
};

}
