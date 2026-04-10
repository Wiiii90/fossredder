/**
 * @file ui/src/controllers/ContractController.cpp
 * @brief Implements the UI controller that forwards contract mutations to the application facade.
 */

#include "ui/controllers/ContractController.h"

#include <algorithm>

#include "core/application/AppStateFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

ContractController::ContractController(core::application::AppStateFacade* core,
                                       QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap ContractController::contract(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& items = core_->state().contracts;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return it != items.end() && *it ? ui::payload::entity::toPayload(**it) : QVariantMap{};
}

QVariantList ContractController::contracts() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().contracts) : QVariantList{};
}

QString ContractController::addContract(const QString& name,
                                        const QString& type,
                                        const QString& description,
                                        const QStringList& actorIds,
                                        const QStringList& propertyIds,
                                        const QStringList& aliases)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::contract::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addContract(
                strings::toStdString(name),
                strings::toStdString(type),
                strings::toStdString(description),
                strings::toStdList(actorIds),
                strings::toStdList(propertyIds),
                strings::toStdList(aliases)));
        });
}

void ContractController::updateContract(const QString& id,
                                        const QString& name,
                                        const QString& type,
                                        const QString& description,
                                        const QStringList& actorIds,
                                        const QStringList& propertyIds,
                                        const QStringList& aliases)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::contract::kUpdate, [&]() {
            core_->updateContract(strings::toStdString(id),
                                  strings::toStdString(name),
                                  strings::toStdString(type),
                                  strings::toStdString(description),
                                  strings::toStdList(actorIds),
                                  strings::toStdList(propertyIds),
                                  strings::toStdList(aliases));
        });
}

void ContractController::deleteContract(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::contract::kDelete,
        [&]() { core_->deleteContract(strings::toStdString(id)); });
}

} // namespace ui
