#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <vector>
#include <memory>

#include "core/models/Property.h"

namespace ui {

class PropertyList : public QAbstractListModel {
    Q_OBJECT

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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setProperties(std::vector<std::shared_ptr<Property>> props);
    const std::vector<std::shared_ptr<Property>>& properties() const;
    int findRowById(const QString& id) const;

    Q_INVOKABLE int addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void removeAt(int row);

private:
    void rebuildIdIndex();

    std::vector<std::shared_ptr<Property>> props_;
    QHash<QString, int> idToRow_;
};

}
