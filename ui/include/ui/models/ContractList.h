/**
 * @file ui/include/ui/models/ContractList.h
 * @brief Declarations for the UI ContractList component.
 */

#pragma once

#include <qqmlintegration.h>

#include "core/models/Contract.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class ContractList : public models::IndexedListModel<core::domain::Contract> {
    Q_OBJECT
    QML_NAMED_ELEMENT(ContractList)
    QML_UNCREATABLE("ContractList is exposed by the application context")
    using Base = models::IndexedListModel<core::domain::Contract>;

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        DescriptionRole,
        StartDateRole,
        EndDateRole,
        BasePriceRole,
        ConsumptionPriceRole,
        MonthlyAdvanceRole,
        ActorIdsRole,
        PropertyIdsRole
    };

    explicit ContractList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setContracts(std::vector<std::shared_ptr<core::domain::Contract>> contracts) { setItems(std::move(contracts)); }
    const std::vector<std::shared_ptr<core::domain::Contract>>& contracts() const { return items(); }
    Q_INVOKABLE int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE int addContract(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
    Q_INVOKABLE QVariantMap get(int index) const;
};

}
