/**
 * @file ui/src/controllers/StatementController.cpp
 * @brief Implements the UI controller that forwards statement mutations to the application facade.
 */

#include "ui/controllers/StatementController.h"

#include <algorithm>

#include "core/application/workspace/WorkspaceFacade.h"
#include "ui/observability/Origins.h"
#include "ui/payload/EntityPayloadMapper.h"
#include "ui/util/CoreFacadeGuard.h"
#include "ui/util/StringConversions.h"

namespace ui {

StatementController::StatementController(core::application::WorkspaceFacade* core,
                                         QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QVariantMap StatementController::statement(const QString& id) const
{
    if (!core_) {
        return {};
    }

    const auto& statements = core_->state().statements;
    const auto statementIt = std::find_if(statements.begin(), statements.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    if (statementIt != statements.end() && *statementIt) {
        return ui::payload::entity::toPayload(**statementIt);
    }

    const auto& transactions = core_->state().transactions;
    const auto transactionIt = std::find_if(transactions.begin(), transactions.end(), [&](const auto& item) {
        return item && QString::fromStdString(item->id) == id;
    });
    return transactionIt != transactions.end() && *transactionIt ? ui::payload::entity::toPayload(**transactionIt) : QVariantMap{};
}

QVariantList StatementController::statements() const
{
    return core_ ? ui::payload::entity::toPayloadList(core_->state().statements) : QVariantList{};
}

QString StatementController::addStatement(const QString& name)
{
    return ui::util::guard::invokeValue<QString>(
        core_, observability::origins::controller::statement::kAdd, {}, [&]() {
            return QString::fromStdString(core_->addStatement(strings::toStdString(name)));
        });
}

void StatementController::updateStatement(const QString& id,
                                          const QString& name)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::statement::kUpdate, [&]() {
            core_->updateStatement(strings::toStdString(id),
                                   strings::toStdString(name));
        });
}


void StatementController::deleteStatement(const QString& id)
{
    ui::util::guard::invokeVoid(
        core_, observability::origins::controller::statement::kDelete,
        [&]() { core_->deleteStatement(strings::toStdString(id)); });
}

} // namespace ui
