/**
 * @file ui/src/controllers/AnnualController.cpp
 * @brief Implements the UI controller that forwards annual aggregate creation to the application facade.
 */

#include "ui/controllers/AnnualController.h"

#include <algorithm>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Origins.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

AnnualController::AnnualController(core::ports::workspace::IWorkspaceWriter* core, QObject* parent)
    : QObject(parent)
    , core_(core)
    , reader_(dynamic_cast<core::ports::workspace::IWorkspaceReader*>(core))
{
}

QVariantMap AnnualController::annual(const QString& id) const
{
    if (!reader_) {
        return {};
    }

    const auto items = reader_->workspaceSnapshot().annuals;
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return QString::fromStdString(item.id) == id;
    });
    if (it == items.end()) {
        return {};
    }

    QVariantMap payload;
    payload[QStringLiteral("id")] = QString::fromStdString(it->id);
    payload[QStringLiteral("name")] = QString::fromStdString(it->name);
    payload[QStringLiteral("year")] = it->year;
    payload[QStringLiteral("analysisIds")] = ui::payload::mapper::toVariantStringList(it->analysisIds);
    payload[QStringLiteral("createdAt")] = QString::fromStdString(it->createdAt);
    payload[QStringLiteral("updatedAt")] = QString::fromStdString(it->updatedAt);
    return payload;
}

QVariantList AnnualController::annuals() const
{
    if (!reader_) {
        return {};
    }

    QVariantList out;
    const auto items = reader_->workspaceSnapshot().annuals;
    out.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        QVariantMap payload;
        payload[QStringLiteral("id")] = QString::fromStdString(item.id);
        payload[QStringLiteral("name")] = QString::fromStdString(item.name);
        payload[QStringLiteral("year")] = item.year;
        payload[QStringLiteral("analysisIds")] = ui::payload::mapper::toVariantStringList(item.analysisIds);
        payload[QStringLiteral("createdAt")] = QString::fromStdString(item.createdAt);
        payload[QStringLiteral("updatedAt")] = QString::fromStdString(item.updatedAt);
        out.push_back(std::move(payload));
    }
    return out;
}

QString AnnualController::addAnnual(const QString& name,
                                    int year,
                                    const QStringList& analysisIds)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::annual::kAdd, {},
        [&]() {
            core::ports::workspace::AnnualCommand command;
            command.name = strings::toStdString(name);
            command.year = year;
            command.assignedAnalysisIds = strings::toStdList(analysisIds);
            return QString::fromStdString(core_->addAnnual(command));
        });
}

void AnnualController::updateAnnual(const QString& id,
                                    const QString& name,
                                    int year,
                                    const QStringList& analysisIds)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::annual::kUpdate,
        [&]() {
            core::ports::workspace::AnnualCommand command;
            command.id = strings::toStdString(id);
            command.name = strings::toStdString(name);
            command.year = year;
            command.assignedAnalysisIds = strings::toStdList(analysisIds);
            core_->updateAnnual(command);
        });
}

QString AnnualController::saveAnnual(const QString& id,
                                     const QString& name,
                                     int year,
                                     const QStringList& analysisIds)
{
    if (id.isEmpty()) {
        return addAnnual(name, year, analysisIds);
    }

    updateAnnual(id, name, year, analysisIds);
    return id;
}

void AnnualController::deleteAnnual(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::annual::kDelete,
        [&]() { core_->deleteAnnual(strings::toStdString(id)); });
}

} // namespace ui
