#include "ui/models/AnnualList.h"

#include <QVariant>

#include "ui/payload/PayloadKeys.h"

namespace ui {

AnnualList::AnnualList(QObject* parent) : Base(parent) {}

QVariant AnnualList::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const auto& a = itemAtRow(index.row());
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
    roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
    roles[YearRole] = ui::payload::keys::annual::kYear.toUtf8();
    roles[VerificationRole] = ui::payload::keys::annual::kVerificationState.toUtf8();
    return roles;
}

}
