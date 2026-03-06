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
    return controllers::guard::invokeValue<QString>(core_, "ui::AnnualController::addAnnual", {}, [&]() {
        return QString::fromStdString(core_->addAnnual(year));
    });
}

void AnnualController::updateAnnual(const QString& id, int year)
{
    controllers::guard::invokeVoid(core_, "ui::AnnualController::updateAnnual", [&]() {
        core_->updateAnnual(id.toStdString(), year);
    });
}

void AnnualController::deleteAnnual(const QString& id)
{
    controllers::guard::invokeVoid(core_, "ui::AnnualController::deleteAnnual", [&]() {
        core_->deleteAnnual(id.toStdString());
    });
}

}
