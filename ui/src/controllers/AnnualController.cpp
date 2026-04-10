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

QString AnnualController::addAnnual(int year)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::annual::kAdd, {},
        [&]() { return QString::fromStdString(core_->addAnnual(year)); });
}

} // namespace ui
