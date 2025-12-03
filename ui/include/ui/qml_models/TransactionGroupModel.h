#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QList>
#include <QVariant>

namespace ui {

class TransactionGroupModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit TransactionGroupModel(QObject* parent = nullptr);

    enum Roles {
        DateRole = Qt::UserRole + 1,
        TransactionsRole
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadDemoData();

private:
    struct Transaction {
        QString id;
        QString label;
        double amount = 0.0;
    };

    struct Group {
        QString date;
        QList<Transaction> transactions;
    };

    QList<Group> m_groups;
};

}
