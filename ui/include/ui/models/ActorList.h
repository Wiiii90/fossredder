#pragma once

#include "core/models/Actor.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class ActorList : public models::IndexedListModel<core::domain::Actor> {
    Q_OBJECT
    using Base = models::IndexedListModel<core::domain::Actor>;

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        DescriptionRole
    };

    explicit ActorList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setActors(std::vector<std::shared_ptr<core::domain::Actor>> actors) { setItems(std::move(actors)); }
    const std::vector<std::shared_ptr<core::domain::Actor>>& actors() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE int addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
};

}
