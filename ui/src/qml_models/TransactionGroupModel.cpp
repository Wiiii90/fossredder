#include "ui/qml_models/TransactionGroupModel.h"

namespace ui {

TransactionGroupModel::TransactionGroupModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TransactionGroupModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_groups.size();
}

QVariant TransactionGroupModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_groups.size()) return QVariant();
    const Group& g = m_groups.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
        case DateRole:
            return g.date;
        case TransactionsRole: {
            QVariantList list;
            for (const Transaction& t : g.transactions) {
                QVariantMap m;
                m["id"] = t.id;
                m["label"] = t.label;
                m["amount"] = t.amount;
                list.append(m);
            }
            return list;
        }
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> TransactionGroupModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[DateRole] = "date";
    roles[TransactionsRole] = "transactions";
    return roles;
}

void TransactionGroupModel::loadDemoData() {
    beginResetModel();
    m_groups.clear();
    Group g1; g1.date = "2025-01-04";
    g1.transactions.append({"tx-1001", "Coffee", 3.5});
    g1.transactions.append({"tx-1002", "Groceries", 24.2});
    m_groups.append(g1);

    Group g2; g2.date = "2025-01-03";
    g2.transactions.append({"tx-2001", "Salary", 2000.0});
    m_groups.append(g2);

    Group g3; g3.date = "2024-12-31";
    g3.transactions.append({"tx-3001", "Rent", 800.0});
    g3.transactions.append({"tx-3002", "Utilities", 120.0});
    m_groups.append(g3);
    endResetModel();
}

}