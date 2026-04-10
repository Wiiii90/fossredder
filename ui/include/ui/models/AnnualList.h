/**
 * @file ui/include/ui/models/AnnualList.h
 * @brief Declarations for the UI AnnualList component.
 */

#pragma once

#include <qqmlintegration.h>

#include "core/models/Annual.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class AnnualList : public models::IndexedListModel<core::domain::Annual> {
    Q_OBJECT
    QML_NAMED_ELEMENT(AnnualList)
    QML_UNCREATABLE("AnnualList is exposed by the application context")
    using Base = models::IndexedListModel<core::domain::Annual>;
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        YearRole,
        VerificationRole
    };

    explicit AnnualList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setAnnuals(std::vector<std::shared_ptr<core::domain::Annual>> annuals) { setItems(std::move(annuals)); }
    const std::vector<std::shared_ptr<core::domain::Annual>>& annuals() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
};

}
