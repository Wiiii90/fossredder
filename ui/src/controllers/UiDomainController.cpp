#include "ui/controllers/UiDomainController.h"

#include <algorithm>
#include <memory>

#include <QUuid>
#include <QTimer>

#include "core/models/Actor.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/import/ActorMatcher.h"
#include "ui/models/StatementDraft.h"

#include <QHash>

UiDomainController::UiDomainController(AppStateController* core, QObject* parent)
    : QObject(parent), core_(core) {}

// Debounced commit helper
void UiDomainController::scheduleDebouncedCommit(const QString& key, int ms) {
    // remove any existing timer
    if (commitTimers_.contains(key)) {
        QTimer* t = commitTimers_.value(key);
        if (t) {
            t->stop();
            t->start(ms);
            return;
        }
    }
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(ms);
    connect(timer, &QTimer::timeout, this, [this, key, timer]() {
        if (core_) { try { core_->commit(); } catch (...) {} }
        commitTimers_.remove(key);
        timer->deleteLater();
    });
    commitTimers_.insert(key, timer);
    timer->start();
}

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
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
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
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
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
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
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
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
    return id;
}

QString UiDomainController::addTransaction(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId)
{
    return addTransactionWithStatus(name, bookingDate, amount, description, statementId, static_cast<int>(Transaction::Status::Neutral));
}

QString UiDomainController::addTransactionWithStatus(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status)
{
    return addTransactionDetailed(name, bookingDate, amount, description, statementId, status, QString());
}

QString UiDomainController::addTransactionDetailed(const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, int status, const QString& actorId)
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
    // Ensure other optional fields are initialized to sensible defaults
    t->valuta.clear();
    t->actorProposal.clear();
    t->metadata.clear();
    t->proofImagePath.clear();
    t->allocatable = false;
    t->propertyIds.clear();
    t->contractId.clear();

    const auto id = QString::fromStdString(t->id);
    core_->mutableState().transactions.push_back(std::move(t));
    scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
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

    // create statement object (leave id empty so repository will assign numeric id)
    auto s = std::make_shared<Statement>();
    s->id.clear();
    s->name = draft->name().toStdString();

    auto& state = core_->mutableState();

    // Build transaction value objects from the draft and persist the statement
    // first so it receives a numeric id. Transactions are stored globally in
    // AppState::transactions and reference the statement via Transaction::statementId.
    const auto& txs = draft->transactions()->drafts();

    // push statement into state and persist so repo assigns id
    state.statements.push_back(s);
    core_->commit();

    // now create global transaction shared_ptrs referencing the statement id
    for (const auto& tx : txs) {
        auto tptr = std::make_shared<Transaction>();
        tptr->id.clear();
        tptr->name = tx.name.toStdString();
        tptr->bookingDate = tx.bookingDate.toStdString();
        tptr->valuta = tx.valuta.toStdString();
        tptr->amount = tx.amount;
        tptr->description = tx.description.toStdString();
        tptr->status = static_cast<Transaction::Status>(tx.status);
        tptr->actorId = tx.actorId.toStdString();
        tptr->metadata = tx.metadata.toStdString();
        tptr->proofImagePath = tx.proofImagePath.toStdString();
        tptr->allocatable = tx.allocatable;
        tptr->propertyIds.clear();
        for (const auto& pid : tx.propertyIds) tptr->propertyIds.push_back(pid.toStdString());
        // Associate with the numeric statement id assigned by the repository
        tptr->statementId = s->id;
        state.transactions.push_back(tptr);
    }

    // notify and persist so UI sees global transactions
    core_->commit();

    return QString::fromStdString(s->id);
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
        scheduleDebouncedCommit(QStringLiteral("actor:") + actorId, 300);
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

            scheduleDebouncedCommit(QStringLiteral("actor:") + id, 300);
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
            scheduleDebouncedCommit(QStringLiteral("property:") + id, 300);
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
            scheduleDebouncedCommit(QStringLiteral("contract:") + id, 300);
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
            scheduleDebouncedCommit(QStringLiteral("statement:") + id, 300);
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

            // Defer commit to the event loop to avoid reentrancy and match other
            // transaction updates (checkboxes/properties use deferred commits).
            if (core_) try { core_->markTransactionDirty(t->id); } catch(...) {}
            scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
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
            if (core_) try { core_->markTransactionDirty(t->id); } catch(...) {}
            scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
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
            if (core_) try { core_->markTransactionDirty(t->id); } catch(...) {}
            scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
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
            if (t->allocatable == allocatable) {
                return;
            }
            t->allocatable = allocatable;
            if (core_) try { core_->markTransactionDirty(t->id); } catch(...) {}
            scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
            return;
        }
    }
}

void UiDomainController::updateTransactionProperties(const QString& id, const QStringList& propertyIds)
{
    if (!core_) return;
    const auto tid = id.toStdString();
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        if (t->id == tid) {
            // build new vector
            std::vector<std::string> newIds;
            newIds.reserve(static_cast<size_t>(propertyIds.size()));
            for (const auto& pid : propertyIds) newIds.push_back(pid.toStdString());

            // if unchanged, ignore to avoid commit loop
            if (t->propertyIds == newIds) return;

            t->propertyIds = std::move(newIds);
            if (core_) try { core_->markTransactionDirty(t->id); } catch(...) {}
            // Debounced commit to coalesce rapid property changes
            scheduleDebouncedCommit(QStringLiteral("tx:") + QString::fromStdString(t->id), 300);
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
            const auto id = QString::fromStdString(a->id);
            return id;
        }
    }
    return {};
}

void UiDomainController::deleteActor(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& actors = core_->mutableState().actors;
    actors.erase(std::remove_if(actors.begin(), actors.end(), [&](const std::shared_ptr<Actor>& a){ return !a || a->id == sid; }), actors.end());
    pruneInvalidContracts();
    scheduleDebouncedCommit(QStringLiteral("actor:") + id, 300);
}

void UiDomainController::deleteProperty(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& props = core_->mutableState().properties;
    props.erase(std::remove_if(props.begin(), props.end(), [&](const std::shared_ptr<Property>& p){ return !p || p->id == sid; }), props.end());

    // remove property references from contracts and transactions
    for (auto& c : core_->mutableState().contracts) {
        if (!c) continue;
        c->propertyIds.erase(std::remove(c->propertyIds.begin(), c->propertyIds.end(), sid), c->propertyIds.end());
        // also clean pointer list (will be rehydrated on load)
        c->properties.erase(std::remove_if(c->properties.begin(), c->properties.end(), [&](Property* p){ return !p || p->id == sid; }), c->properties.end());
    }
    for (auto& t : core_->mutableState().transactions) {
        if (!t) continue;
        t->propertyIds.erase(std::remove(t->propertyIds.begin(), t->propertyIds.end(), sid), t->propertyIds.end());
    }
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
}

void UiDomainController::deleteContract(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& vec = core_->mutableState().contracts;
    vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const std::shared_ptr<Contract>& c){ return !c || c->id == sid; }), vec.end());
    scheduleDebouncedCommit(QStringLiteral("contract:") + id, 300);
}

void UiDomainController::deleteStatement(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    // remove statement
    auto& stmts = core_->mutableState().statements;
    stmts.erase(std::remove_if(stmts.begin(), stmts.end(), [&](const std::shared_ptr<Statement>& s){ return !s || s->id == sid; }), stmts.end());
    // remove transactions that reference this statement
    auto& txs = core_->mutableState().transactions;
    txs.erase(std::remove_if(txs.begin(), txs.end(), [&](const std::shared_ptr<Transaction>& t){ return !t || t->statementId == sid; }), txs.end());
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
}

void UiDomainController::deleteTransaction(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& txs = core_->mutableState().transactions;
    txs.erase(std::remove_if(txs.begin(), txs.end(), [&](const std::shared_ptr<Transaction>& t){ return !t || t->id == sid; }), txs.end());
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
}

QString UiDomainController::ensurePropertyByName(const QString& name)
{
    if (!core_) return {};
    if (name.trimmed().isEmpty()) return {};
    for (const auto& p : core_->state().properties) {
        if (p && QString::fromStdString(p->name).trimmed().compare(name.trimmed(), Qt::CaseInsensitive) == 0) {
            return QString::fromStdString(p->id);
        }
    }
    auto prop = std::make_shared<Property>();
    prop->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    prop->name = name.toStdString();
    core_->mutableState().properties.push_back(prop);
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
    return QString::fromStdString(core_->mutableState().properties.back()->id);
}

QString UiDomainController::ensureStatementByName(const QString& name)
{
    if (!core_) return {};
    if (name.trimmed().isEmpty()) return {};
    for (const auto& s : core_->state().statements) {
        if (s && QString::fromStdString(s->name).trimmed().compare(name.trimmed(), Qt::CaseInsensitive) == 0) {
            return QString::fromStdString(s->id);
        }
    }
    auto s = std::make_shared<Statement>();
    s->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    s->name = name.toStdString();
    core_->mutableState().statements.push_back(s);
    scheduleDebouncedCommit(QStringLiteral("save"), 300);
    return QString::fromStdString(core_->mutableState().statements.back()->id);
}

void UiDomainController::pruneInvalidContracts()
{
    if (!core_) return;
    auto& cs = core_->mutableState().contracts;
    cs.erase(std::remove_if(cs.begin(), cs.end(), [](const std::shared_ptr<Contract>& c){
        if (!c) return true;
        if (c->name.empty()) return true;
        if (c->actorIds.empty() && c->propertyIds.empty() && c->actors.empty() && c->properties.empty()) return true;
        return false;
    }), cs.end());
}

void UiDomainController::pruneInvalidTransactions()
{
    if (!core_) return;
    auto& txs = core_->mutableState().transactions;
    txs.erase(std::remove_if(txs.begin(), txs.end(), [](const std::shared_ptr<Transaction>& t){
        if (!t) return true;
        // prune transactions that have no statement and no name
        if (t->statementId.empty() && t->name.empty()) return true;
        return false;
    }), txs.end());
}
