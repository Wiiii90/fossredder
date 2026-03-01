#include "ui/models/AnnualList.h"

#include <QVariant>

namespace ui {

AnnualList::AnnualList(QObject* parent) : QAbstractListModel(parent) {}

int AnnualList::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(annuals_.size());
}

QVariant AnnualList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(annuals_.size())) return {};
    const auto& a = annuals_[row];
    if (!a) return {};

    switch (role) {
    case IdRole: return QString::fromStdString(a->id);
    case YearRole: return a->year;
    case VerificationRole: return static_cast<int>(a->verificationState);
    default: return {};
    }
}

QHash<int, QByteArray> AnnualList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[YearRole] = "year";
    roles[VerificationRole] = "verificationState";
    return roles;
}

void AnnualList::setAnnuals(std::vector<std::shared_ptr<Annual>> annuals) {
    beginResetModel();
    annuals_ = std::move(annuals);
    endResetModel();
}

std::vector<std::shared_ptr<Annual>> AnnualList::annuals() const {
    return annuals_;
}

void AnnualList::removeAt(int row) {
    if (row < 0 || row >= static_cast<int>(annuals_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    annuals_.erase(annuals_.begin() + row);
    endRemoveRows();
}

}
