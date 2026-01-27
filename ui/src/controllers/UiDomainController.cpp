#include "ui/controllers/UiDomainController.h"

#include <algorithm>
#include <memory>

#include <QUuid>

#include "core/models/Actor.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/models/Analysis.h"
#include "core/import/ActorMatcher.h"
#include "ui/models/StatementDraft.h"

#include "core/controllers/ExportController.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/XlsxController.h"
#include "core/analysis/AnalysisController.h"
#include <QVariant>
#include "core/analysis/Filter.h"

#include <QHash>
#include <cstdio>

static std::string q2s(const QString& s) { auto u8 = s.toUtf8(); return std::string(u8.constData(), static_cast<size_t>(u8.size())); }

UiDomainController::UiDomainController(AppStateController* core, QObject* parent)
    : QObject(parent), core_(core) {}

// Note: analysisController_ is lazily constructed when used to avoid header ordering issues

// Debounced commit logic removed. Persistence must be invoked explicitly
// via the AppStateController::commit() call from higher-level code when
// a durable save is desired.

// removed heuristic assignment - contracts are created explicitly from draft types

QString UiDomainController::addActor(const QString& name, const QString& type, const QString& description) { return addActorWithAliases(name, type, description, {}); }

QString UiDomainController::addActorWithAliases(const QString& name, const QString& type, const QString& description, const QStringList& aliases) {
    if (!core_) return {};
    auto actor = std::make_shared<Actor>();
    actor->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    actor->name = q2s(name);
    actor->type = q2s(type);
    actor->description = q2s(description);
    actor->aliases.clear(); actor->aliases.reserve(static_cast<size_t>(aliases.size()));
    for (const auto& a: aliases) { auto t = a.trimmed(); if (t.isEmpty()) continue; actor->aliases.push_back(q2s(t)); }
    core_->mutableState().actors.push_back(actor);
    return QString::fromStdString(actor->id);
}

QStringList UiDomainController::getPropertyIds() const {
    QStringList out;
    if (!core_) return out;
    for (const auto& p : core_->state().properties) {
        if (!p) continue;
        out.push_back(QString::fromStdString(p->id));
    }
    try { fprintf(stderr, "UiDomainController::getPropertyIds: returning %d ids\n", out.size()); } catch(...) {}
    return out;
}

QStringList UiDomainController::getContractTypes() const {
    QStringList out;
    if (!core_) return out;
    std::unordered_set<std::string> seen;
    for (const auto& c : core_->state().contracts) {
        if (!c) continue;
        const std::string t = c->type;
        if (t.empty()) continue;
        if (seen.find(t) != seen.end()) continue;
        seen.insert(t);
        out.push_back(QString::fromStdString(t));
    }
    try { fprintf(stderr, "UiDomainController::getContractTypes: returning %d types\n", out.size()); } catch(...) {}
    for (const auto &s : out) { try { fprintf(stderr, " UiDomainController::getContractTypes: type=%s\n", s.toUtf8().constData()); } catch(...) {} }
    return out;
}

QStringList UiDomainController::getActorAliases(const QString& actorId) const {
    if (!core_) return {};
    const auto id = actorId.toStdString();
    for (const auto& a : core_->state().actors) {
        if (!a) continue;
        if (a->id != id) continue;
        QStringList out;
        for (const auto& al : a->aliases) out.push_back(QString::fromStdString(al));
        return out;
    }
    return {};
}

void UiDomainController::setActorAliases(const QString& actorId, const QStringList& aliases) {
    if (!core_) return;
    const auto id = actorId.toStdString();
        for (auto& a : core_->mutableState().actors) {
        if (!a) continue;
        if (a->id != id) continue;
        a->aliases.clear();
        a->aliases.reserve(static_cast<size_t>(aliases.size()));
        for (const auto& al : aliases) {
            const auto t = al.trimmed();
            if (t.isEmpty()) continue;
            a->aliases.push_back(q2s(t));
        }
        return;
    }
}

QString UiDomainController::addProperty(const QString& name, const QString& address, const QString& description) {
    if (!core_) return {};
    auto p = std::make_shared<Property>();
    p->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    p->name = q2s(name);
    p->address = q2s(address);
    p->description = q2s(description);
    core_->mutableState().properties.push_back(p);
    if (core_) core_->notifyState();
    return QString::fromStdString(p->id);
}

QString UiDomainController::addContract(const QString& name, const QString& type, const QString& description,
                                       const QStringList& actorIds, const QStringList& propertyIds) {
    if (!core_) return {};
    auto c = std::make_shared<Contract>();
    c->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    c->name = q2s(name);
    c->type = q2s(type);
    c->description = q2s(description);
    c->actorIds.clear(); for (const auto& aid : actorIds) c->actorIds.push_back(aid.toStdString());
    c->propertyIds.clear(); for (const auto& pid : propertyIds) c->propertyIds.push_back(pid.toStdString());
    core_->mutableState().contracts.push_back(c);
    return QString::fromStdString(c->id);
}

QString UiDomainController::addStatement(const QString& name) {
    if (!core_) return {};
    auto s = std::make_shared<Statement>();
    s->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    s->name = q2s(name);
    core_->mutableState().statements.push_back(s);
    return QString::fromStdString(s->id);
}

QString UiDomainController::addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec) {
    if (!core_) return {};
    auto analysis = std::make_shared<Analysis>();
    analysis->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    analysis->name = q2s(name);
    analysis->type = q2s(type);
    analysis->configJson = q2s(configJson);
    analysis->filterSpec = q2s(filterSpec);
    core_->mutableState().analyses.push_back(analysis);
    if (core_) core_->notifyState();
    return QString::fromStdString(analysis->id);
}

QVariantMap UiDomainController::computeAnalysis(const QString& analysisId, const QString& filterSpec) const {
    QVariantMap out;
    if (!core_) return out;
    const std::string aid = analysisId.toStdString();
    const AnalysisController ctrl;
    // find analysis
    for (const auto& a : core_->state().analyses) {
        if (!a) continue;
        if (a->id != aid) continue;
        // determine effective filterSpec: prefer explicit param, else analysis.filterSpec
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
        // include analysis type/config for QML rendering — prefer concrete plot subtype if provided
        std::string outType = a->type;
        std::string outConfig = a->configJson;
        // if analysis is a plot and configJson contains a plotType field, extract it for UI convenience
        if (outType == "plot" && !outConfig.empty()) {
            // try a simple substring parse for "plotType" : "..."
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

        // also include matched transactions (ids + summary) according to filter
        QVariantList txlist;
        Filter f = parseFilterSpec(effectiveFilter);
        try { fprintf(stderr, "UiDomainController::computeAnalysis: parsing filter='%s' f.empty=%d totalTx=%zu\n", effectiveFilter.c_str(), f.empty(), core_->state().transactions.size()); } catch(...) {}
        size_t matched = 0;
        for (const auto& tptr : core_->state().transactions) {
            if (!tptr) continue;
            bool match = true;
            if (!effectiveFilter.empty()) match = f.matches(tptr, core_->state());
            try { fprintf(stderr, "  tx id=%s date='%s' contractId='%s' match=%d\n", tptr->id.c_str(), tptr->bookingDate.c_str(), tptr->contractId.c_str(), match); } catch(...) {}
            if (!match) continue;
            QVariantMap tm;
            tm["id"] = QString::fromStdString(tptr->id);
            tm["name"] = QString::fromStdString(tptr->name);
            tm["date"] = QString::fromStdString(tptr->bookingDate);
            tm["amount"] = tptr->amount;
            tm["contractId"] = QString::fromStdString(tptr->contractId);
            // include resolved contract type for easier UI filtering/legend matching
            std::string ctype = "unassigned";
            if (!tptr->contractId.empty()) {
                for (const auto& cptr : core_->state().contracts) {
                    if (!cptr) continue;
                    if (cptr->id == tptr->contractId) { ctype = cptr->type; break; }
                }
            }
            tm["contractType"] = QString::fromStdString(ctype);
            txlist.push_back(tm);
            ++matched;
        }
        try { fprintf(stderr, "UiDomainController::computeAnalysis: matched=%zu transactions\n", matched); } catch(...) {}
        out["transactions"] = txlist;
        out["artifacts"] = artifacts;
        out["generatedAt"] = QString::fromStdString(res.generatedAt);

        // debug: print summary to stderr to help UI troubleshooting
        try {
            fprintf(stderr, "UiDomainController::computeAnalysis: analysisId=%s effectiveFilter='%s' tableRows=%zu metrics=%zu matchedTx=%zu\n",
                    a->id.c_str(), effectiveFilter.c_str(), res.table.size(), res.metrics.size(), txlist.size());
        } catch(...) {}
        return out;
    }

    return out;
}

QString UiDomainController::addTransaction(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId) {
    return addTransactionWithStatus(name, bookingDate, amount, description, statementId, static_cast<int>(Transaction::Status::Neutral));
}

QString UiDomainController::addTransactionWithStatus(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status) {
    return addTransactionDetailed(name, bookingDate, amount, description, statementId, status, QString());
}

QString UiDomainController::addTransactionDetailed(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status, const QString& actorId) {
    if (!core_) return {};
    if (statementId.trimmed().isEmpty()) return {};
    auto t = std::make_shared<Transaction>();
    t->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    t->name = q2s(name);
    t->bookingDate = q2s(bookingDate);
    t->amount = amount;
    t->description = q2s(description);
    t->statementId = statementId.toStdString();
    t->status = static_cast<Transaction::Status>(status);
    t->actorId = actorId.toStdString();
    t->valuta.clear(); t->actorProposal.clear(); t->metadata.clear(); t->proofImagePath.clear(); t->allocatable = false;
    core_->mutableState().transactions.push_back(t);
    if (core_) {
        try {
            fprintf(stderr, "UiDomainController::addTransactionDetailed: added tx id='%s' stmt='%s' totalTx=%zu\n", t->id.c_str(), t->statementId.c_str(), core_->state().transactions.size());
        } catch(...) {}
    }
    return QString::fromStdString(t->id);
}

QString UiDomainController::finalizeStatementDraft(StatementDraft* draft) {
    if (!core_ || !draft) return {};

    const auto& txs = draft->transactions()->drafts();
    if (txs.empty()) return {};

    // create statement
    const QString stmtName = draft->name().trimmed();
    const QString statementId = addStatement(stmtName.isEmpty() ? QStringLiteral("Imported") : stmtName);
    if (statementId.isEmpty()) return {};

    // create transactions from draft and attach properties/actor/status
    for (const auto& d : txs) {
        // create transaction with status and actor if provided
        const QString name = d.name;
        const QString bookingDate = d.bookingDate;
        const double amount = d.amount;
        const QString description = d.description;
        const int status = d.status;
        const QString actorId = d.actorId;

        const QString txId = addTransactionWithStatus(name, bookingDate, amount, description, statementId, status);
        if (txId.isEmpty()) continue;

        // apply actor if present
        if (!actorId.isEmpty()) updateTransactionActor(txId, actorId);

        // allocatable
        updateTransactionAllocatable(txId, d.allocatable);

        // properties (if any)
        if (!d.propertyIds.isEmpty()) updateTransactionProperties(txId, d.propertyIds);

        // If draft provided a type, always create a new contract with that type and attach it.
        if (!d.type.trimmed().isEmpty()) {
            const QString requestedType = d.type.trimmed();
            QStringList props = d.propertyIds;
            // generate a sequential contract name like "Vertrag 1", "Vertrag 2", ...
            int maxIdx = 0;
            const std::string prefix = "Vertrag ";
            for (const auto& cptr : core_->state().contracts) {
                if (!cptr) continue;
                const std::string& nm = cptr->name;
                if (nm.size() > prefix.size() && nm.rfind(prefix, 0) == 0) {
                    std::string rest = nm.substr(prefix.size());
                    try {
                        int v = std::stoi(rest);
                        if (v > maxIdx) maxIdx = v;
                    } catch (...) {}
                }
            }
            QString newName = QStringLiteral("Vertrag %1").arg(maxIdx + 1);
            QString createdId = addContract(newName, requestedType, QString(), QStringList(), props);
        if (!createdId.isEmpty()) {
                for (auto& tptr : core_->mutableState().transactions) {
                    if (!tptr) continue;
                    if (QString::fromStdString(tptr->id) != txId) continue;
                    tptr->contractId = createdId.toStdString();
                    break;
                }
            }
        }
    }

    // Notify UI about the newly added statement/transactions (in-memory only)
    if (core_) {
        core_->notifyState();
        // Persist immediately so imported statements/transactions are durable
        try { core_->commit(); } catch (...) {}
    }

    return statementId;
}

bool UiDomainController::tryAutoAssignActorForDraftTransaction(StatementDraft* draft, int index) {
    if (!core_ || !draft) return false;
    const auto& actors = core_->state().actors;
    if (actors.empty()) return false;
    const auto& txs = draft->transactions()->drafts();
    if (index < 0 || index >= static_cast<int>(txs.size())) return false;
    const auto& tx = txs[static_cast<size_t>(index)];
    if (!tx.actorId.trimmed().isEmpty()) return false;
    std::string text;
    const auto prop = tx.actorProposal.trimmed();
    if (!prop.isEmpty()) text = q2s(prop); else { text = q2s(tx.description); if (text.empty()) text = q2s(tx.name); }
    ActorMatcher matcher;
    auto match = matcher.match(text, actors);
    if (!match.hasMatch()) return false;
    draft->transactions()->setActorId(index, QString::fromStdString(match.actorId));
    return true;
}

int UiDomainController::autoAssignActorsForDraft(StatementDraft* draft) {
    if (!draft) return 0;
    int count = 0;
    const auto& txs = draft->transactions()->drafts();
    for (int i = 0; i < static_cast<int>(txs.size()); ++i) if (tryAutoAssignActorForDraftTransaction(draft, i)) ++count;
    return count;
}

bool UiDomainController::canFinalizeStatementDraft(StatementDraft* draft) const {
    if (!draft) return false;
    const auto& txs = draft->transactions()->drafts();
    return !txs.empty();
}

void UiDomainController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description) {
    if (!core_) return; const auto sid = id.toStdString();
        for (auto& a: core_->mutableState().actors) { if (!a) continue; if (a->id != sid) continue; a->name = q2s(name); a->type = q2s(type); a->description = q2s(description); return; }
}

void UiDomainController::updateActorWithAliases(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases) { updateActor(id,name,type,description); setActorAliases(id,aliases); }

void UiDomainController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description) { if (!core_) return; const auto sid=id.toStdString(); for (auto& p: core_->mutableState().properties) { if (!p) continue; if (p->id!=sid) continue; p->name=q2s(name); p->address=q2s(address); p->description=q2s(description); return; } }

void UiDomainController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                       const QStringList& actorIds, const QStringList& propertyIds) { if (!core_) return; const auto sid=id.toStdString(); for (auto& c: core_->mutableState().contracts) { if (!c) continue; if (c->id!=sid) continue; c->name=q2s(name); c->type=q2s(type); c->description=q2s(description); c->actorIds.clear(); for (const auto& a: actorIds) c->actorIds.push_back(a.toStdString()); c->propertyIds.clear(); for (const auto& p: propertyIds) c->propertyIds.push_back(p.toStdString()); return; } }

void UiDomainController::updateStatement(const QString& id, const QString& name) { if (!core_) return; const auto sid=id.toStdString(); for (auto& s: core_->mutableState().statements) { if (!s) continue; if (s->id!=sid) continue; s->name=q2s(name); return; } }

void UiDomainController::updateTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId) { if (!core_) return; const auto sid=id.toStdString(); for (auto& t: core_->mutableState().transactions) { if (!t) continue; if (t->id!=sid) continue; t->name=q2s(name); t->bookingDate=q2s(bookingDate); t->amount=amount; t->description=q2s(description); t->statementId=statementId.toStdString(); return; } }

void UiDomainController::updateTransactionStatus(const QString& id, int status) { if (!core_) return; const auto sid=id.toStdString(); for (auto& t: core_->mutableState().transactions) { if (!t) continue; if (t->id!=sid) continue; t->status = static_cast<Transaction::Status>(status); return; } }

void UiDomainController::updateTransactionActor(const QString& id, const QString& actorId) { if (!core_) return; const auto sid=id.toStdString(); for (auto& t: core_->mutableState().transactions) { if (!t) continue; if (t->id!=sid) continue; t->actorId=actorId.toStdString(); return; } }

void UiDomainController::updateTransactionAllocatable(const QString& id, bool allocatable) { if (!core_) return; const auto sid=id.toStdString(); for (auto& t: core_->mutableState().transactions) { if (!t) continue; if (t->id!=sid) continue; t->allocatable=allocatable; return; } }

void UiDomainController::updateTransactionProperties(const QString& id, const QStringList& propertyIds) { if (!core_) return; const auto sid=id.toStdString(); for (auto& t: core_->mutableState().transactions) { if (!t) continue; if (t->id!=sid) continue; t->propertyIds.clear(); for (const auto& p: propertyIds) t->propertyIds.push_back(p.toStdString()); return; } }

void UiDomainController::exportData(const QString& format, bool includeFormulas, const QString& path) {
    if (!core_) return;
    auto csv = std::make_shared<core::controllers::exporting::CsvController>();
    auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
    core::controllers::exporting::ExportController exporter(xlsx,csv);
    core::controllers::exporting::ExportOptions opts;
    opts.outputPath = path.toStdString(); opts.includeFormulas = includeFormulas; opts.locale = q2s(format); opts.state = &core_->state();
    if (format.toLower() == "csv") opts.format = core::controllers::exporting::ExportOptions::Format::Csv; else opts.format = core::controllers::exporting::ExportOptions::Format::Xlsx;
    exporter.exportData(opts);
}

void UiDomainController::deleteActor(const QString& id) { if (!core_) return; const auto sid=id.toStdString(); auto& v = core_->mutableState().actors; v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& a){ return !a ? false : a->id==sid; }), v.end()); }
void UiDomainController::deleteProperty(const QString& id) { if (!core_) return; const auto sid=id.toStdString(); auto& v = core_->mutableState().properties; v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& p){ return !p ? false : p->id==sid; }), v.end()); }
void UiDomainController::deleteContract(const QString& id) { if (!core_) return; const auto sid=id.toStdString(); auto& v = core_->mutableState().contracts; v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& c){ return !c ? false : c->id==sid; }), v.end()); }
void UiDomainController::deleteStatement(const QString& id) { if (!core_) return; const auto sid=id.toStdString(); auto& v = core_->mutableState().statements; v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& s){ return !s ? false : s->id==sid; }), v.end()); }
void UiDomainController::deleteTransaction(const QString& id) { if (!core_) return; const auto sid=id.toStdString(); auto& v = core_->mutableState().transactions; v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& t){ return !t ? false : t->id==sid; }), v.end()); }

QString UiDomainController::ensureActorByName(const QString& name) { if (!core_) return {}; for (const auto& a : core_->state().actors) { if (!a) continue; if (QString::fromStdString(a->name) == name) return QString::fromStdString(a->id); } return addActor(name, QString(), QString()); }
QString UiDomainController::ensurePropertyByName(const QString& name) { if (!core_) return {}; for (const auto& p : core_->state().properties) { if (!p) continue; if (QString::fromStdString(p->name) == name) return QString::fromStdString(p->id); } return addProperty(name, QString(), QString()); }
QString UiDomainController::ensureStatementByName(const QString& name) { if (!core_) return {}; for (const auto& s : core_->state().statements) { if (!s) continue; if (QString::fromStdString(s->name) == name) return QString::fromStdString(s->id); } return addStatement(name); }

void UiDomainController::commit() {
    if (!core_) return;
    try { core_->commit(); } catch (...) {}
}
