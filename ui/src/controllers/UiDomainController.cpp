#include "ui/controllers/UiDomainController.h"

#include <algorithm>
#include <memory>

#include <QUuid>

#include "core/models/Actor.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/import/ActorMatcher.h"
#include "ui/models/StatementDraft.h"

UiDomainController::UiDomainController(AppStateController* core, QObject* parent)
    : QObject(parent), core_(core) {}

QString UiDomainController::addActor(const QString& name, const QString& type, const QString& description)
{
    return addActorWithAliases(name, type, description, {});
}

QString UiDomainController::addActorWithAliases(const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return {};

    auto actor = std::make_shared<Actor>();
    actor->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    actor->name = name.toStdString();
    actor->type = type.toStdString();
    actor->description = description.toStdString();

    actor->aliases.clear();
    actor->aliases.reserve(static_cast<size_t>(aliases.size()));
    for (const auto& a : aliases) {
        const auto t = a.trimmed();
        if (t.isEmpty()) continue;
        actor->aliases.push_back(t.toStdString());
    }

    const auto id = QString::fromStdString(actor->id);
    core_->mutableState().actors.push_back(std::move(actor));
    core_->commit();
    return id;
}

QString UiDomainController::addProperty(const QString& name, const QString& address, const QString& description)
{
    if (!core_) return {};

    auto prop = std::make_shared<Property>();
    prop->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    prop->name = name.toStdString();
    prop->address = address.toStdString();
    prop->description = description.toStdString();

    const auto id = QString::fromStdString(prop->id);
    core_->mutableState().properties.push_back(std::move(prop));
    core_->commit();
    return id;
}

QString UiDomainController::addContract(const QString& name, const QString& type, const QString& description,
                                       const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return {};

    auto c = std::make_shared<Contract>();
    c->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    c->name = name.toStdString();
    c->type = type.toStdString();
    c->description = description.toStdString();

    c->actorIds.clear();
    for (const auto& aid : actorIds) c->actorIds.push_back(aid.toStdString());

    c->propertyIds.clear();
    for (const auto& pid : propertyIds) c->propertyIds.push_back(pid.toStdString());

    const auto id = QString::fromStdString(c->id);
    core_->mutableState().contracts.push_back(std::move(c));
    core_->commit();
    return id;
}

QString UiDomainController::addStatement(const QString& name)
{
    if (!core_) return {};

    auto s = std::make_shared<Statement>();
    s->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    s->name = name.toStdString();

    const auto id = QString::fromStdString(s->id);
    core_->mutableState().statements.push_back(std::move(s));
    core_->commit();
    return id;
}

QString UiDomainController::addTransaction(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId)
{
    return addTransactionWithStatus(name, bookingDate, amount, description, statementId, static_cast<int>(Transaction::Status::Neutral));
}

QString UiDomainController::addTransactionWithStatus(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status)
{
    return addTransactionWithStatusAndActor(name, bookingDate, amount, description, statementId, status, QString());
}

QString UiDomainController::addTransactionWithStatusAndActor(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status, const QString& actorId)
{
    if (!core_) return {};
    if (statementId.trimmed().isEmpty()) return {};

    auto t = std::make_shared<Transaction>();
    t->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    t->name = name.toStdString();
    t->bookingDate = bookingDate.toStdString();
    t->amount = amount;
    t->description = description.toStdString();
    t->statementId = statementId.toStdString();
    t->status = static_cast<Transaction::Status>(status);
    t->actorId = actorId.toStdString();

    const auto id = QString::fromStdString(t->id);
    core_->mutableState().transactions.push_back(std::move(t));
    pruneInvalidTransactions();
    core_->commit();
    return id;
}

static std::string q2s(const QString& s)
{
    auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}

bool UiDomainController::tryAutoAssignActorForDraftTransaction(StatementDraft* draft, int index)
{
    if (!core_ || !draft) return false;
    const auto& actors = core_->state().actors;
    if (actors.empty()) return false;

    const auto& txs = draft->transactions()->drafts();
    if (index < 0 || index >= static_cast<int>(txs.size())) return false;

    const auto& tx = txs[static_cast<size_t>(index)];
    if (!tx.actorId.trimmed().isEmpty()) return false;

    std::string text;
    const auto prop = tx.actorProposal.trimmed();
    if (!prop.isEmpty()) {
        text = q2s(prop);
    } else {
        text = q2s(tx.description);
        if (text.empty()) text = q2s(tx.name);
    }

    ActorMatcher matcher;
    auto match = matcher.match(text, actors);
    if (!match.hasMatch()) return false;

    draft->transactions()->setActorId(index, QString::fromStdString(match.actorId));
    return true;
}

int UiDomainController::autoAssignActorsForDraft(StatementDraft* draft)
{
    if (!draft) return 0;
    int count = 0;
    const auto& txs = draft->transactions()->drafts();
    for (int i = 0; i < static_cast<int>(txs.size()); ++i) {
        if (tryAutoAssignActorForDraftTransaction(draft, i)) ++count;
    }
    return count;
}

bool UiDomainController::canFinalizeStatementDraft(StatementDraft* draft) const
{
    if (!draft) return false;
    const auto& txs = draft->transactions()->drafts();
    // Allow finalizing even if some transactions have no actor assigned.
    // Require at least one transaction to be present.
    return !txs.empty();
}

QString UiDomainController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!core_ || !draft) return {};

    autoAssignActorsForDraft(draft);
    if (!canFinalizeStatementDraft(draft)) return {};

    const QString sid = addStatement(draft->name());
    if (sid.isEmpty()) return {};

    auto& state = core_->mutableState();

    const auto& txs = draft->transactions()->drafts();
    for (const auto& tx : txs) {
        auto t = std::make_shared<Transaction>();
        t->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        t->name = tx.name.toStdString();
        t->bookingDate = tx.bookingDate.toStdString();
        t->valuta = tx.valuta.toStdString();
        t->amount = tx.amount;
        t->description = tx.description.toStdString();
        t->statementId = sid.toStdString();
        t->status = static_cast<Transaction::Status>(tx.status);
        t->actorId = tx.actorId.toStdString();

        t->metadata = tx.metadata.toStdString();
        t->proofImagePath = tx.proofImagePath.toStdString();

        // Preserve allocatable flag from draft
        t->allocatable = tx.allocatable;

        // Preserve property assignments from draft
        t->propertyIds.clear();
        for (const auto& pid : tx.propertyIds) t->propertyIds.push_back(pid.toStdString());

        state.transactions.push_back(std::move(t));
    }

    pruneInvalidTransactions();
    core_->commit();

    return sid;
}

QStringList UiDomainController::getActorAliases(const QString& actorId) const
{
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

void UiDomainController::setActorAliases(const QString& actorId, const QStringList& aliases)
{
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
            a->aliases.push_back(t.toStdString());
        }
        core_->commit();
        return;
    }
}

void UiDomainController::updateActor(const QString& id, const QString& name, const QString& type, const QString& description)
{
    updateActorWithAliases(id, name, type, description, getActorAliases(id));
}

void UiDomainController::updateActorWithAliases(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& a : core_->mutableState().actors) {
        if (!a) continue;
        if (a->id == sid) {
            a->name = name.toStdString();
            a->type = type.toStdString();
            a->description = description.toStdString();

            a->aliases.clear();
            a->aliases.reserve(static_cast<size_t>(aliases.size()));
            for (const auto& al : aliases) {
                const auto t = al.trimmed();
                if (t.isEmpty()) continue;
                a->aliases.push_back(t.toStdString());
            }

            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& p : core_->mutableState().properties) {
        if (!p) continue;
        if (p->id == sid) {
            p->name = name.toStdString();
            p->address = address.toStdString();
            p->description = description.toStdString();
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                       const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& c : core_->mutableState().contracts) {
        if (!c) continue;
        if (c->id == sid) {
            c->name = name.toStdString();
            c->type = type.toStdString();
            c->description = description.toStdString();

            c->actorIds.clear();
            for (const auto& aid : actorIds) c->actorIds.push_back(aid.toStdString());

            c->propertyIds.clear();
            for (const auto& pid : propertyIds) c->propertyIds.push_back(pid.toStdString());

            pruneInvalidContracts();
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateStatement(const QString& id, const QString& name)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& s : core_->mutableState().statements) {
        if (!s) continue;
        if (s->id == sid) {
            s->name = name.toStdString();
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->id == sid) {
            t->name = name.toStdString();
            t->bookingDate = bookingDate.toStdString();
            t->amount = amount;
            t->description = description.toStdString();
            t->statementId = statementId.toStdString();
            pruneInvalidTransactions();
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateTransactionStatus(const QString& id, int status)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->id == sid) {
            t->status = static_cast<Transaction::Status>(status);
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateTransactionActor(const QString& id, const QString& actorId)
{
    if (!core_) return;

    const auto tid = id.toStdString();
    const auto aid = actorId.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->id == tid) {
            t->actorId = aid;
            core_->commit();
            return;
        }
    }
}

void UiDomainController::updateTransactionAllocatable(const QString& id, bool allocatable)
{
    if (!core_) return;

    const auto tid = id.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->id == tid) {
            t->allocatable = allocatable;
            core_->commit();
            return;
        }
    }
}

static bool eqName(const std::string& a, const QString& b)
{
    return QString::fromStdString(a).trimmed().compare(b.trimmed(), Qt::CaseInsensitive) == 0;
}

QString UiDomainController::ensureActorByName(const QString& name)
{
    if (!core_) return {};
    if (name.trimmed().isEmpty()) return {};

    for (const auto& a : core_->state().actors) {
        if (a && eqName(a->name, name)) {
            return QString::fromStdString(a->id);
        }
    }

    return addActor(name, QString(), QString());
}

QString UiDomainController::ensurePropertyByName(const QString& name)
{
    if (!core_) return {};
    if (name.trimmed().isEmpty()) return {};

    for (const auto& p : core_->state().properties) {
        if (p && eqName(p->name, name)) {
            return QString::fromStdString(p->id);
        }
    }

    return addProperty(name, QString(), QString());
}

QString UiDomainController::ensureStatementByName(const QString& name)
{
    if (!core_) return {};
    if (name.trimmed().isEmpty()) return {};

    for (const auto& s : core_->state().statements) {
        if (s && eqName(s->name, name)) {
            return QString::fromStdString(s->id);
        }
    }

    return addStatement(name);
}

void UiDomainController::deleteContract(const QString& id)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    auto& cs = core_->mutableState().contracts;
    cs.erase(std::remove_if(cs.begin(), cs.end(), [&](const auto& c) {
        return c && c->id == sid;
    }), cs.end());

    core_->commit();
}

void UiDomainController::deleteActor(const QString& id)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    auto& as = core_->mutableState().actors;
    as.erase(std::remove_if(as.begin(), as.end(), [&](const auto& a) {
        return a && a->id == sid;
    }), as.end());

    for (auto& c : core_->mutableState().contracts) {
        if (!c) continue;
        c->actorIds.erase(std::remove(c->actorIds.begin(), c->actorIds.end(), sid), c->actorIds.end());
    }

    pruneInvalidContracts();
    core_->commit();
}

void UiDomainController::deleteProperty(const QString& id)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    auto& ps = core_->mutableState().properties;
    ps.erase(std::remove_if(ps.begin(), ps.end(), [&](const auto& p) {
        return p && p->id == sid;
    }), ps.end());

    for (auto& c : core_->mutableState().contracts) {
        if (!c) continue;
        c->propertyIds.erase(std::remove(c->propertyIds.begin(), c->propertyIds.end(), sid), c->propertyIds.end());
    }

    pruneInvalidContracts();
    core_->commit();
}

void UiDomainController::deleteTransaction(const QString& id)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    auto& ts = core_->mutableState().transactions;
    ts.erase(std::remove_if(ts.begin(), ts.end(), [&](const auto& t) {
        return t && t->id == sid;
    }), ts.end());

    core_->commit();
}

void UiDomainController::deleteStatement(const QString& id)
{
    if (!core_) return;

    const auto sid = id.toStdString();
    auto& ss = core_->mutableState().statements;
    ss.erase(std::remove_if(ss.begin(), ss.end(), [&](const auto& s) {
        return s && s->id == sid;
    }), ss.end());

    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->statementId == sid) {
            t->statementId.clear();
        }
    }

    pruneInvalidTransactions();
    core_->commit();
}

void UiDomainController::pruneInvalidContracts()
{
    auto& cs = core_->mutableState().contracts;
    cs.erase(std::remove_if(cs.begin(), cs.end(), [](const auto& c) {
        if (!c) return true;
        return c->actorIds.empty() || c->propertyIds.empty();
    }), cs.end());
}

void UiDomainController::pruneInvalidTransactions()
{
    auto& ts = core_->mutableState().transactions;
    ts.erase(std::remove_if(ts.begin(), ts.end(), [](const auto& t) {
        if (!t) return true;
        return t->statementId.empty();
    }), ts.end());
}
