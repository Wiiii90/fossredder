/**
 * @file ui/include/ui/models/ActorList.h
 * @brief Declarations for the UI ActorList component.
 */

#pragma once

#include <qqmlintegration.h>

#include "core/models/Actor.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class ActorList : public models::IndexedListModel<core::domain::Actor> {
    Q_OBJECT
    QML_NAMED_ELEMENT(ActorList)
    QML_UNCREATABLE("ActorList is exposed by the application context")
    using Base = models::IndexedListModel<core::domain::Actor>;

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit ActorList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setActors(std::vector<std::shared_ptr<core::domain::Actor>> actors) { setItems(std::move(actors)); }
    const std::vector<std::shared_ptr<core::domain::Actor>>& actors() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE int addActor(const QString& name);
    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
};

}
