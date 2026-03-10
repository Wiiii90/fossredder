#pragma once

#include <QObject>

#include "ui/models/RowListModel.h"
#include "ui/models/TransactionDraft.h"

namespace ui {

class TransactionDraftList : public models::RowListModel<TransactionDraft> {
    Q_OBJECT
    using Base = models::RowListModel<TransactionDraft>;
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        BookingDateRole,
        ValutaRole,
        AmountRole,
        DescriptionRole,
        ActorIdRole,
        ActorProposalRole,
        MetadataRole,
        ProofImagePathRole,
        AllocatableRole,
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
    Q_INVOKABLE void setActorProposal(int index, const QString& actorProposal);

    Q_INVOKABLE void setName(int index, const QString& name);
    Q_INVOKABLE void setBookingDate(int index, const QString& bookingDate);
    Q_INVOKABLE void setValuta(int index, const QString& valuta);
    Q_INVOKABLE void setAmount(int index, double amount);
    Q_INVOKABLE void setDescription(int index, const QString& description);

    Q_INVOKABLE void setMetadata(int index, const QString& v);
    Q_INVOKABLE void setProofImagePath(int index, const QString& v);

    Q_INVOKABLE void setStatus(int index, int status);
    Q_INVOKABLE void setAllocatable(int index, bool v);

    Q_INVOKABLE void setProperties(int index, const QStringList& ids);
    Q_INVOKABLE void setType(int index, const QString& v);

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
