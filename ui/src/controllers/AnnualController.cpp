#include "ui/controllers/AnnualController.h"

#include "ui/controllers/ControllerGuard.h"

namespace ui {

AnnualController::AnnualController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString AnnualController::addAnnual(int year)
{
    if (!controllers::guard::ensureCore(core_, "ui::AnnualController::addAnnual")) return {};
    try {
        return QString::fromStdString(core_->addAnnual(year));
    } catch (...) {
        controllers::guard::reportException("ui::AnnualController::addAnnual");
    }
    return {};
}

void AnnualController::updateAnnual(const QString& id, int year)
{
    if (!controllers::guard::ensureCore(core_, "ui::AnnualController::updateAnnual")) return;
    try {
        core_->updateAnnual(id.toStdString(), year);
    } catch (...) {
        controllers::guard::reportException("ui::AnnualController::updateAnnual");
    }
}

void AnnualController::deleteAnnual(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::AnnualController::deleteAnnual")) return;
    try {
        core_->deleteAnnual(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::AnnualController::deleteAnnual");
    }
}

}
