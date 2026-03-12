#include "ui/controllers/DraftController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"
#include "ui/observability/Origins.h"

#include "core/models/DraftStatement.h"

namespace ui {

DraftController::DraftController(core::controllers::AppStateController *core, QObject *parent)
    : QObject(parent), core_(core) {}

QString DraftController::finalizeStatementDraft(StatementDraft *draft) {
  if (!draft)
    return {};
  return controllers::guard::invokeValue<QString>(
      core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
        const auto &drafts = draft->transactions()->drafts();
        if (drafts.empty())
          return QString();

        DraftStatement input;
        input.name = strings::toStdString(draft->name());
        input.transactions.reserve(drafts.size());
        for (const auto &d : drafts) {
          DraftTransaction transaction;
          transaction.name = strings::toStdString(d.name);
          transaction.bookingDate = strings::toStdString(d.bookingDate);
          transaction.amount = d.amount;
          transaction.description = strings::toStdString(d.description);
          transaction.status = static_cast<Transaction::Status>(d.status);
          transaction.actorId = strings::toStdString(d.actorId);
          transaction.allocatable = d.allocatable;
          transaction.propertyIds = strings::toStdList(d.propertyIds);
          transaction.type = strings::toStdString(d.type);
          input.transactions.push_back(std::move(transaction));
        }

        return QString::fromStdString(core_->finalizeStatementDraft(input));
      });
}

} // namespace ui
