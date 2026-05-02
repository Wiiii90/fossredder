/**
 * @file ui/include/ui/models/PropertyList.h
 * @brief Declarations for the UI PropertyList component.
 */

#pragma once

#include <qqmlintegration.h>

#include "core/models/Property.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class PropertyList : public models::IndexedListModel<core::domain::Property> {
    Q_OBJECT
    QML_NAMED_ELEMENT(PropertyList)
    QML_UNCREATABLE("PropertyList is exposed by the application context")
    using Base = models::IndexedListModel<core::domain::Property>;

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        AddressRole,
        DescriptionRole,
        ConsumptionRole,
        ConsumptionUnitRole
    };

    explicit PropertyList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setProperties(std::vector<std::shared_ptr<core::domain::Property>> props) { setItems(std::move(props)); }
    const std::vector<std::shared_ptr<core::domain::Property>>& properties() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE int addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
};

}
