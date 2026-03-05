#include "ui/controllers/DomainController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"

#include "core/models/DraftStatement.h"
#include "core/models/Analysis.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <unordered_set>

namespace ui {

DomainController::DomainController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString DomainController::addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addActor")) return {};
    try {
        return QString::fromStdString(core_->addActor(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases)));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addActor");
    }
    return {};
}

void DomainController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateActor")) return;
    try {
        core_->updateActor(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateActor");
    }
}

void DomainController::deleteActor(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteActor")) return;
    try {
        core_->deleteActor(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteActor");
    }
}

QString DomainController::addAnnual(int year)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addAnnual")) return {};
    try {
        return QString::fromStdString(core_->addAnnual(year));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addAnnual");
    }
    return {};
}

void DomainController::updateAnnual(const QString& id, int year)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateAnnual")) return;
    try {
        core_->updateAnnual(id.toStdString(), year);
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateAnnual");
    }
}

void DomainController::deleteAnnual(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteAnnual")) return;
    try {
        core_->deleteAnnual(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteAnnual");
    }
}

QString DomainController::addContract(const QString& name, const QString& type, const QString& description,
                                      const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addContract")) return {};
    try {
        return QString::fromStdString(core_->addContract(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds)));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addContract");
    }
    return {};
}

void DomainController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                      const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateContract")) return;
    try {
        core_->updateContract(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateContract");
    }
}

void DomainController::deleteContract(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteContract")) return;
    try {
        core_->deleteContract(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteContract");
    }
}

QString DomainController::addProperty(const QString& name, const QString& address, const QString& description)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addProperty")) return {};
    try {
        return QString::fromStdString(core_->addProperty(strings::toStdString(name), strings::toStdString(address), strings::toStdString(description)));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addProperty");
    }
    return {};
}

void DomainController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateProperty")) return;
    try {
        core_->updateProperty(id.toStdString(), strings::toStdString(name), strings::toStdString(address), strings::toStdString(description));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateProperty");
    }
}

void DomainController::deleteProperty(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteProperty")) return;
    try {
        core_->deleteProperty(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteProperty");
    }
}

QString DomainController::addStatement(const QString& name)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addStatement")) return {};
    try {
        return QString::fromStdString(core_->addStatement(strings::toStdString(name)));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addStatement");
    }
    return {};
}

void DomainController::updateStatement(const QString& id, const QString& name)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateStatement")) return;
    try {
        core_->updateStatement(id.toStdString(), strings::toStdString(name));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateStatement");
    }
}

void DomainController::deleteStatement(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteStatement")) return;
    try {
        core_->deleteStatement(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteStatement");
    }
}

QString DomainController::addTransaction(const QString& name,
                                         const QString& bookingDate,
                                         double amount,
                                         const QString& description,
                                         const QString& statementId,
                                         int status,
                                         const QString& actorId,
                                         bool allocatable,
                                         const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::addTransaction")) return {};
    try {
        return QString::fromStdString(core_->addTransaction(strings::toStdString(name),
                                                             strings::toStdString(bookingDate),
                                                             amount,
                                                             strings::toStdString(description),
                                                             statementId.toStdString(),
                                                             status,
                                                             actorId.toStdString(),
                                                             allocatable,
                                                             strings::toStdList(propertyIds)));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::addTransaction");
    }
    return {};
}

void DomainController::updateTransaction(const QString& id,
                                         const QString& name,
                                         const QString& bookingDate,
                                         double amount,
                                         const QString& description,
                                         const QString& statementId,
                                         int status,
                                         const QString& actorId,
                                         bool allocatable,
                                         const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::updateTransaction")) return;
    try {
        core_->updateTransaction(id.toStdString(),
                                 strings::toStdString(name),
                                 strings::toStdString(bookingDate),
                                 amount,
                                 strings::toStdString(description),
                                 statementId.toStdString(),
                                 status,
                                 actorId.toStdString(),
                                 allocatable,
                                 strings::toStdList(propertyIds));
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::updateTransaction");
    }
}

void DomainController::deleteTransaction(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::deleteTransaction")) return;
    try {
        core_->deleteTransaction(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::deleteTransaction");
    }
}

QStringList DomainController::getContractTypes() const
{
    QStringList out;
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::getContractTypes")) return out;
    try {
        std::unordered_set<std::string> seen;
        for (const auto& c : core_->state().contracts) {
            if (!c) continue;
            const std::string type = c->type;
            if (type.empty()) continue;
            if (seen.find(type) != seen.end()) continue;
            seen.insert(type);
            out.push_back(QString::fromStdString(type));
        }
    } catch (...) {
        controllers::guard::reportException("ui::DomainController::getContractTypes");
    }
    return out;
}

QString DomainController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!controllers::guard::ensureCore(core_, "ui::DomainController::finalizeStatementDraft") || !draft) return {};

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
        controllers::guard::reportException("ui::DomainController::finalizeStatementDraft");
    }
    return {};
}

}
