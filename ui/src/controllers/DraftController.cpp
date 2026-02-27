#include "ui/controllers/DraftController.h"

#include <memory>

#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "ui/models/StatementDraft.h"
#include "ui/controllers/StatementController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/TransactionController.h"

DraftController::DraftController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!core_ || !draft) return {};

    const auto& drafts = draft->transactions()->drafts();
    if (drafts.empty()) return {};

    ui::StatementController statementCtrl(core_, this);
    ContractController contractCtrl(core_, this);
    TransactionController txCtrl(core_, this);

    const QString statementName = draft->name().trimmed();
    const QString statementId = statementCtrl.addStatement(statementName.isEmpty() ? QStringLiteral("Imported") : statementName);
    if (statementId.isEmpty()) return {};

    for (const auto& d : drafts) {
        const QString txId = txCtrl.addTransaction(d.name,
                                                   d.bookingDate,
                                                   d.amount,
                                                   d.description,
                                                   statementId,
                                                   d.status,
                                                   d.actorId,
                                                   d.allocatable,
                                                   d.propertyIds);
        if (txId.isEmpty()) continue;

        if (!d.type.trimmed().isEmpty()) {
            int maxIdx = 0;
            constexpr const char* prefix = "Vertrag ";
            for (const auto& cptr : core_->state().contracts) {
                if (!cptr) continue;
                const std::string& nm = cptr->name;
                if (nm.size() <= std::char_traits<char>::length(prefix) || nm.rfind(prefix, 0) != 0) continue;
                const std::string rest = nm.substr(std::char_traits<char>::length(prefix));
                try {
                    const int v = std::stoi(rest);
                    if (v > maxIdx) maxIdx = v;
                } catch (...) {}
            }
            const QString contractName = QStringLiteral("Vertrag %1").arg(maxIdx + 1);
            const QString contractId = contractCtrl.addContract(contractName, d.type.trimmed(), QString(), {}, d.propertyIds);
            if (!contractId.isEmpty()) {
                const auto txStdId = txId.toStdString();
                for (auto& t : core_->mutableState().transactions) {
                    if (!t || t->id != txStdId) continue;
                    t->contractId = contractId.toStdString();
                    break;
                }
            }
        }
    }

    core_->notifyState();
    core_->commit();
    return statementId;
}
