/**
 * @file ui/include/ui/models/ImportRunList.h
 * @brief Declarations for the UI ImportRunList component.
 */

#pragma once

#include <QString>

#include "ui/models/RowListModel.h"

namespace ui {

struct ImportRunRow {
    QString time;
    QString type;
    QString file;
    QString status;
    QString message;
};

class ImportRunList : public models::RowListModel<ImportRunRow> {
    Q_OBJECT
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        TypeRole,
        FileRole,
        StatusRole,
        MessageRole
    };

private:
    using Base = models::RowListModel<ImportRunRow>;

public:

    explicit ImportRunList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRun(QString time, QString type, QString file, QString status, QString message);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();
};

}
