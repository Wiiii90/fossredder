/**
 * @file ui/src/controllers/PropertyController.cpp
 * @brief Implements the UI controller that forwards property mutations to the application facade.
 */

#include "ui/controllers/PropertyController.h"

#include <algorithm>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Origins.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

PropertyController::PropertyController(core::ports::workspace::IWorkspaceWriter* core,
                                       QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
{
}

QVariantMap PropertyController::property(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    const auto items = reader_->workspaceSnapshot().properties;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (it == items.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(it->id);
    payload[QStringLiteral("name")] = QString::fromStdString(it->name);
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

QVariantList PropertyController::properties() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().properties;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
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

QString PropertyController::addProperty(const QString& name,
                                        const QStringList& aliases)
{
    return ui::util::guard::invokeValue<QString>(core_, observability::origins::controller::property::kAdd, {}, [&]() {
        core::ports::workspace::PropertyCommand command;
        command.name = strings::toStdString(name);
        for (const auto& alias : strings::toAliases(aliases)) {
            command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
        }
        return QString::fromStdString(core_->addProperty(command));
    });
}

void PropertyController::updateProperty(const QString& id,
                                        const QString& name,
                                        const QStringList& aliases)
{
    ui::util::guard::invokeVoid(core_, observability::origins::controller::property::kUpdate, [&]() {
        core::ports::workspace::PropertyCommand command;
        command.id = strings::toStdString(id);
        command.name = strings::toStdString(name);
        for (const auto& alias : strings::toAliases(aliases)) {
            command.aliases.push_back({alias.value(), alias.kind(), alias.source(), alias.hitCount(), alias.lastUsedAt(), alias.createdAt(), alias.updatedAt()});
        }
        core_->updateProperty(command);
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
