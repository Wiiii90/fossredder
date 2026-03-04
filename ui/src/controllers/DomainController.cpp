#include "ui/controllers/DomainController.h"

#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"

#include "core/analysis/AnalysisController.h"
#include "core/analysis/Filter.h"
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
    if (!core_) return {};
    return QString::fromStdString(core_->addActor(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases)));
}

void DomainController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return;
    core_->updateActor(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdListTrimmed(aliases));
}

void DomainController::deleteActor(const QString& id)
{
    if (!core_) return;
    core_->deleteActor(id.toStdString());
}

QString DomainController::addAnnual(int year)
{
    if (!core_) return {};
    return QString::fromStdString(core_->addAnnual(year));
}

void DomainController::updateAnnual(const QString& id, int year)
{
    if (!core_) return;
    core_->updateAnnual(id.toStdString(), year);
}

void DomainController::deleteAnnual(const QString& id)
{
    if (!core_) return;
    core_->deleteAnnual(id.toStdString());
}

QString DomainController::addContract(const QString& name, const QString& type, const QString& description,
                                      const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return {};
    return QString::fromStdString(core_->addContract(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds)));
}

void DomainController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                      const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return;
    core_->updateContract(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds));
}

void DomainController::deleteContract(const QString& id)
{
    if (!core_) return;
    core_->deleteContract(id.toStdString());
}

QString DomainController::addProperty(const QString& name, const QString& address, const QString& description)
{
    if (!core_) return {};
    return QString::fromStdString(core_->addProperty(strings::toStdString(name), strings::toStdString(address), strings::toStdString(description)));
}

void DomainController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!core_) return;
    core_->updateProperty(id.toStdString(), strings::toStdString(name), strings::toStdString(address), strings::toStdString(description));
}

void DomainController::deleteProperty(const QString& id)
{
    if (!core_) return;
    core_->deleteProperty(id.toStdString());
}

QString DomainController::addStatement(const QString& name)
{
    if (!core_) return {};
    return QString::fromStdString(core_->addStatement(strings::toStdString(name)));
}

void DomainController::updateStatement(const QString& id, const QString& name)
{
    if (!core_) return;
    core_->updateStatement(id.toStdString(), strings::toStdString(name));
}

void DomainController::deleteStatement(const QString& id)
{
    if (!core_) return;
    core_->deleteStatement(id.toStdString());
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
    if (!core_) return {};
    return QString::fromStdString(core_->addTransaction(strings::toStdString(name),
                                                         strings::toStdString(bookingDate),
                                                         amount,
                                                         strings::toStdString(description),
                                                         statementId.toStdString(),
                                                         status,
                                                         actorId.toStdString(),
                                                         allocatable,
                                                         strings::toStdList(propertyIds)));
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
    if (!core_) return;
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
}

void DomainController::deleteTransaction(const QString& id)
{
    if (!core_) return;
    core_->deleteTransaction(id.toStdString());
}

QStringList DomainController::getContractTypes() const
{
    QStringList out;
    if (!core_) return out;
    std::unordered_set<std::string> seen;
    for (const auto& c : core_->state().contracts) {
        if (!c) continue;
        const std::string type = c->type;
        if (type.empty()) continue;
        if (seen.find(type) != seen.end()) continue;
        seen.insert(type);
        out.push_back(QString::fromStdString(type));
    }
    return out;
}

QString DomainController::addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec)
{
    if (!core_) return {};
    return QString::fromStdString(core_->addAnalysis(strings::toStdString(name), strings::toStdString(type), strings::toStdString(configJson), strings::toStdString(filterSpec)));
}

QVariantMap DomainController::computeAnalysis(const QString& analysisId, const QString& filterSpec) const
{
    QVariantMap out;
    if (!core_) return out;

    const std::string aid = analysisId.toStdString();
    const ::AnalysisController ctrl;

    for (const auto& a : core_->state().analyses) {
        if (!a) continue;
        if (a->id != aid) continue;

        const std::string effectiveFilter = filterSpec.isEmpty() ? a->filterSpec : filterSpec.toStdString();
        const auto res = ctrl.computeAnalysis(*a, core_->state(), effectiveFilter);

        QVariantMap metrics;
        for (const auto& kv : res.metrics) metrics.insert(QString::fromStdString(kv.first), kv.second);

        QVariantList table;
        for (const auto& row : res.table) {
            QVariantList r;
            for (const auto& col : row) r.push_back(QString::fromStdString(col));
            table.push_back(r);
        }

        QVariantList artifacts;
        for (const auto& art : res.artifacts) artifacts.push_back(QString::fromStdString(art));

        out["metrics"] = metrics;
        out["table"] = table;

        std::string outType = a->type;
        std::string outConfig = a->configJson;
        if (outType == "plot" && !outConfig.empty()) {
            const std::string key = "\"plotType\"";
            auto pos = outConfig.find(key);
            if (pos != std::string::npos) {
                auto colon = outConfig.find(':', pos + key.size());
                if (colon != std::string::npos) {
                    auto firstQuote = outConfig.find('"', colon + 1);
                    if (firstQuote != std::string::npos) {
                        auto secondQuote = outConfig.find('"', firstQuote + 1);
                        if (secondQuote != std::string::npos && secondQuote > firstQuote) {
                            outType = outConfig.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                        }
                    }
                }
            }
        }

        out["type"] = QString::fromStdString(outType);
        out["config"] = QString::fromStdString(outConfig);

        QVariantList txlist;
        Filter f = parseFilterSpec(effectiveFilter);
        for (const auto& tptr : core_->state().transactions) {
            if (!tptr) continue;
            bool match = true;
            if (!effectiveFilter.empty()) match = f.matches(tptr, core_->state());
            if (!match) continue;
            QVariantMap tm;
            tm["id"] = QString::fromStdString(tptr->id);
            tm["name"] = QString::fromStdString(tptr->name);
            tm["date"] = QString::fromStdString(tptr->bookingDate);
            tm["amount"] = tptr->amount;
            tm["contractId"] = QString::fromStdString(tptr->contractId);

            std::string contractType = "unassigned";
            if (!tptr->contractId.empty()) {
                for (const auto& cptr : core_->state().contracts) {
                    if (!cptr) continue;
                    if (cptr->id == tptr->contractId) { contractType = cptr->type; break; }
                }
            }
            tm["contractType"] = QString::fromStdString(contractType);
            txlist.push_back(tm);
        }

        out["transactions"] = txlist;
        out["artifacts"] = artifacts;
        out["generatedAt"] = QString::fromStdString(res.generatedAt);
        return out;
    }

    return out;
}

QString DomainController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!core_ || !draft) return {};

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
}

}
