#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <memory>

#include "core/models/Annual.h"

namespace ui {

class AnnualList : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        YearRole,
        VerificationRole
    };

    explicit AnnualList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setAnnuals(std::vector<std::shared_ptr<Annual>> annuals);
    std::vector<std::shared_ptr<Annual>> annuals() const;

    Q_INVOKABLE void removeAt(int row);

private:
    std::vector<std::shared_ptr<Annual>> annuals_;
};

}
