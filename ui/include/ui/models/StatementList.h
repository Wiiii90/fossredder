#pragma once

#include <QAbstractListModel>
#include <QString>

#include <memory>
#include <vector>

#include "core/models/Statement.h"

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
    std::vector<std::shared_ptr<Statement>> statements() const;
    Q_INVOKABLE void removeAt(int row);

private:
    std::vector<std::shared_ptr<Statement>> statements_;
};
