/**
 * @file ui/src/controllers/ActorController.cpp
 * @brief Implements the UI controller that forwards actor mutations to the application facade.
 */

#include "ui/controllers/ActorController.h"

#include <algorithm>

#include "core/application/workspace/WorkspaceFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

ActorController::ActorController(core::application::WorkspaceFacade* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap ActorController::actor(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& items = core_->state().actors;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

QVariantList ActorController::actors() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().actors) : QVariantList{};
}

QString ActorController::addActor(const QString& name,
                                  const QStringList& aliases)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::actor::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addActor(
                strings::toStdString(name),
                strings::toAliases(aliases)));
        });
}

void ActorController::updateActor(const QString& id,
                                  const QString& name,
                                  const QStringList& aliases)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::actor::kUpdate, [&]() {
            core_->updateActor(strings::toStdString(id),
                               strings::toStdString(name),
                               strings::toAliases(aliases));
        });
}

QString ActorController::saveActor(const QString& id,
                                   const QString& name,
                                   const QStringList& aliases)
{
    if (id.isEmpty()) {
        return addActor(name, aliases);
    }

    updateActor(id, name, aliases);
    return id;
}

void ActorController::deleteActor(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::actor::kDelete,
        [&]() { core_->deleteActor(strings::toStdString(id)); });
}

} // namespace ui
