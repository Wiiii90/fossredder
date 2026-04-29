/**
 * @file ui/src/controllers/AnnualController.cpp
 * @brief Implements the UI controller that forwards annual aggregate creation to the application facade.
 */

#include "ui/controllers/AnnualController.h"

#include <algorithm>

#include "core/application/AppStateFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

AnnualController::AnnualController(core::application::AppStateFacade* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap AnnualController::annual(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& items = core_->state().annuals;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

QVariantList AnnualController::annuals() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().annuals) : QVariantList{};
}

QString AnnualController::addAnnual(const QString& name,
                                    int year,
                                    const QStringList& assignedAnalysisIds)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::annual::kAdd, {},
        [&]() {
            return QString::fromStdString(
                core_->addAnnual(strings::toStdString(name),
                                 year,
                                 strings::toStdList(assignedAnalysisIds)));
        });
}

void AnnualController::updateAnnual(const QString& id,
                                    const QString& name,
                                    int year,
                                    const QStringList& assignedAnalysisIds)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::annual::kUpdate,
        [&]() {
            core_->updateAnnual(strings::toStdString(id),
                                strings::toStdString(name),
                                year,
                                strings::toStdList(assignedAnalysisIds));
        });
}

QString AnnualController::saveAnnual(const QString& id,
                                     const QString& name,
                                     int year,
                                     const QStringList& assignedAnalysisIds)
{
    if (id.isEmpty()) {
        return addAnnual(name, year, assignedAnalysisIds);
    }

    updateAnnual(id, name, year, assignedAnalysisIds);
    return id;
}

void AnnualController::deleteAnnual(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::annual::kDelete,
        [&]() { core_->deleteAnnual(strings::toStdString(id)); });
}

} // namespace ui
