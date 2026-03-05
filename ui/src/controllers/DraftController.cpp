#include "ui/controllers/DraftController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"

#include "core/models/DraftStatement.h"

namespace ui {

DraftController::DraftController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!controllers::guard::ensureCore(core_, "ui::DraftController::finalizeStatementDraft") || !draft) return {};

    try {
        const auto& drafts = draft->transactions()->drafts();
        if (drafts.empty()) return {};

        DraftStatement input;
        input.name = strings::toStdString(draft->name());
        input.transactions.reserve(drafts.size());
        for (const auto& d : drafts) {
            DraftTransaction transaction;
            transaction.name = strings::toStdString(d.name);
            transaction.bookingDate = strings::toStdString(d.bookingDate);
            transaction.amount = d.amount;
            transaction.description = strings::toStdString(d.description);
            transaction.status = d.status;
            transaction.actorId = d.actorId.toStdString();
            transaction.allocatable = d.allocatable;
            transaction.propertyIds = strings::toStdList(d.propertyIds);
            transaction.type = strings::toStdString(d.type);
            input.transactions.push_back(std::move(transaction));
        }

        return QString::fromStdString(core_->finalizeStatementDraft(input));
    } catch (...) {
        controllers::guard::reportException("ui::DraftController::finalizeStatementDraft");
    }
    return {};
}

}
