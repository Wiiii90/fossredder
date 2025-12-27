#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <memory>

#include "core/models/Property.h"

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
    std::vector<std::shared_ptr<Property>> properties() const;

    Q_INVOKABLE int addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void removeAt(int row);

private:
    std::vector<std::shared_ptr<Property>> props_;
};
