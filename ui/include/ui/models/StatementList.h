#pragma once

#include "core/models/Statement.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class StatementList : public models::IndexedListModel<Statement> {
    Q_OBJECT
    using Base = models::IndexedListModel<Statement>;
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit StatementList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setStatements(std::vector<std::shared_ptr<Statement>> statements) { setItems(std::move(statements)); }
    const std::vector<std::shared_ptr<Statement>>& statements() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }
    Q_INVOKABLE void removeAt(int row) { removeItemAt(row); }
};

}
