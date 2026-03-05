#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>

#include <memory>
#include <vector>

#include "core/models/Statement.h"

namespace ui {

class StatementList : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit StatementList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setStatements(std::vector<std::shared_ptr<Statement>> statements);
    const std::vector<std::shared_ptr<Statement>>& statements() const;
    int findRowById(const QString& id) const;
    Q_INVOKABLE void removeAt(int row);

private:
    void rebuildIdIndex();

    std::vector<std::shared_ptr<Statement>> statements_;
    QHash<QString, int> idToRow_;
};

}
