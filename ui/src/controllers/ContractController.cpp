/**
 * @file ui/src/controllers/ContractController.cpp
 * @brief Implements the UI controller that forwards contract mutations to the application facade.
 */

#include "ui/controllers/ContractController.h"

#include <algorithm>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Origins.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

ContractController::ContractController(core::ports::workspace::IWorkspaceWriter* core,
                                       QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
{
}

QVariantMap ContractController::contract(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    const auto items = reader_->workspaceSnapshot().contracts;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (it == items.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(it->id);
    payload[QStringLiteral("name")] = QString::fromStdString(it->name);
    payload[QStringLiteral("type")] = QString::fromStdString(it->type);
    payload[QStringLiteral("actorIds")] = ui::payload::mapper::toVariantStringList(it->actorIds);
    payload[QStringLiteral("propertyIds")] = ui::payload::mapper::toVariantStringList(it->propertyIds);
    QVariantList aliases;
    aliases.reserve(static_cast<int>(it->aliases.size()));
    for (const auto& alias : it->aliases) {
        QVariantMap aliasPayload;
        aliasPayload[QStringLiteral("value")] = QString::fromStdString(alias.value);
        aliasPayload[QStringLiteral("kind")] = QString::fromStdString(alias.kind);
        aliasPayload[QStringLiteral("source")] = QString::fromStdString(alias.source);
        aliasPayload[QStringLiteral("createdAt")] = QString::fromStdString(alias.createdAt);
        aliasPayload[QStringLiteral("updatedAt")] = QString::fromStdString(alias.updatedAt);
        aliases.push_back(std::move(aliasPayload));
    }
    payload[QStringLiteral("aliases")] = aliases;
    payload[QStringLiteral("createdAt")] = QString::fromStdString(it->createdAt);
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(it->updatedAt);
    return payload;
}

QVariantList ContractController::contracts() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().contracts;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
        payload[QStringLiteral("type")] = QString::fromStdString(item.type);
        payload[QStringLiteral("actorIds")] = ui::payload::mapper::toVariantStringList(item.actorIds);
        payload[QStringLiteral("propertyIds")] = ui::payload::mapper::toVariantStringList(item.propertyIds);
        QVariantList aliases;
        aliases.reserve(static_cast<int>(item.aliases.size()));
        for (const auto& alias : item.aliases) {
            QVariantMap aliasPayload;
            aliasPayload[QStringLiteral("value")] = QString::fromStdString(alias.value);
            aliasPayload[QStringLiteral("kind")] = QString::fromStdString(alias.kind);
            aliasPayload[QStringLiteral("source")] = QString::fromStdString(alias.source);
            aliasPayload[QStringLiteral("createdAt")] = QString::fromStdString(alias.createdAt);
            aliasPayload[QStringLiteral("updatedAt")] = QString::fromStdString(alias.updatedAt);
            aliases.push_back(std::move(aliasPayload));
        }
        payload[QStringLiteral("aliases")] = aliases;
        payload[QStringLiteral("createdAt")] = QString::fromStdString(item.createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(item.updatedAt);
        out.push_back(std::move(payload));
    }
    return out;
}

QString ContractController::addContract(const QString& name,
                                        const QString& type,
                                        const QStringList& actorIds,
                                        const QStringList& propertyIds,
                                        const QStringList& aliases)
{
    return ui::util::guard::invokeValue<QString>(core_, observability::origins::controller::contract::kAdd, {}, [&]() {
        core::ports::workspace::ContractCommand command;
        command.name = strings::toStdString(name);
        command.type = strings::toStdString(type);
        command.actorIds = strings::toStdList(actorIds);
        command.propertyIds = strings::toStdList(propertyIds);
        for (const auto& alias : strings::toAliases(aliases)) {
            command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
        }
        return QString::fromStdString(core_->addContract(command));
    });
}

void ContractController::updateContract(const QString& id,
                                        const QString& name,
                                        const QString& type,
                                        const QStringList& actorIds,
                                        const QStringList& propertyIds,
                                        const QStringList& aliases)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::contract::kUpdate, [&]() {
        core::ports::workspace::ContractCommand command;
        command.id = strings::toStdString(id);
        command.name = strings::toStdString(name);
        command.type = strings::toStdString(type);
        command.actorIds = strings::toStdList(actorIds);
        command.propertyIds = strings::toStdList(propertyIds);
        for (const auto& alias : strings::toAliases(aliases)) {
            command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
        }
        core_->updateContract(command);
    });
}

QString ContractController::saveContract(const QString& id,
                                         const QString& name,
                                         const QString& type,
                                         const QStringList& actorIds,
                                         const QStringList& propertyIds,
                                         const QStringList& aliases)
{
    if (id.isEmpty()) {
        return addContract(name, type, actorIds, propertyIds, aliases);
    }

    updateContract(id, name, type, actorIds, propertyIds, aliases);
    return id;
}

void ContractController::deleteContract(const QString& id)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::contract::kDelete, [&]() {
        core_->deleteContract(strings::toStdString(id));
    });
}

} // namespace ui
