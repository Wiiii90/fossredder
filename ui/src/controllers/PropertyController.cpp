/**
 * @file ui/src/controllers/PropertyController.cpp
 * @brief Implements the UI controller that forwards property mutations to the application facade.
 */

#include "ui/controllers/PropertyController.h"

#include <algorithm>

#include "core/application/workspace/WorkspaceFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

PropertyController::PropertyController(core::application::WorkspaceFacade* core,
                                       QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap PropertyController::property(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& items = core_->state().properties;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

QVariantList PropertyController::properties() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().properties) : QVariantList{};
}

QString PropertyController::addProperty(const QString& name,
                                        const QStringList& aliases)
{
    return ui::util::guard::invokeValue<QString>(core_, observability::origins::controller::property::kAdd, {}, [&]() {
        return QString::fromStdString(core_->addProperty(strings::toStdString(name), strings::toAliases(aliases)));
    });
}

void PropertyController::updateProperty(const QString& id,
                                        const QString& name,
                                        const QStringList& aliases)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::property::kUpdate, [&]() {
        core_->updateProperty(strings::toStdString(id), strings::toStdString(name), strings::toAliases(aliases));
    });
}

QString PropertyController::saveProperty(const QString& id,
                                         const QString& name,
                                         const QStringList& aliases)
{
    if (id.isEmpty()) {
        return addProperty(name, aliases);
    }

    updateProperty(id, name, aliases);
    return id;
}

void PropertyController::deleteProperty(const QString& id)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::property::kDelete, [&]() {
        core_->deleteProperty(strings::toStdString(id));
    });
}

} // namespace ui
