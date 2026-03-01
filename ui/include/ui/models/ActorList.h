#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <memory>

#include "core/models/Actor.h"

namespace ui {

class ActorList : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        DescriptionRole
    };

    explicit ActorList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setActors(std::vector<std::shared_ptr<Actor>> actors);
    std::vector<std::shared_ptr<Actor>> actors() const;

    Q_INVOKABLE int addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void removeAt(int row);

private:
    std::vector<std::shared_ptr<Actor>> actors_;
};

}
