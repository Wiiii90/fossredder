#include "ui/controllers/AnnualController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Annual.h"

AnnualController::AnnualController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString AnnualController::addAnnual(int year)
{
    if (!core_) return {};
    auto annual = std::make_shared<Annual>();
    annual->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    annual->year = year;
    core_->mutableState().annuals.push_back(annual);
    core_->notifyState();
    return QString::fromStdString(annual->id);
}

void AnnualController::updateAnnual(const QString& id, int year)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& a : core_->mutableState().annuals) {
        if (!a || a->id != sid) continue;
        a->year = year;
        core_->notifyState();
        return;
    }
}

void AnnualController::deleteAnnual(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& v = core_->mutableState().annuals;
    v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& a) { return a && a->id == sid; }), v.end());
    core_->notifyState();
}
