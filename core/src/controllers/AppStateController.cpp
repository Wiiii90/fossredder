/**
 * @file core/src/controllers/AppStateController.cpp
 * @brief Implementation of AppStateController.
 *
 * Implements the runtime behaviour of the AppStateController which manages the
 * in-memory AppState and delegates persistence to an IStorageManager.
 */

#include "core/pch.h"
#include "core/controllers/AppStateController.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <random>

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

namespace {

std::string generateId()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    static const char* hex = "0123456789abcdef";
    std::uniform_int_distribution<int> dist(0, 15);
    std::string out;
    out.reserve(32);
    for (int i = 0; i < 32; ++i) out.push_back(hex[dist(rng)]);
    return out;
}

std::string trimCopy(const std::string& value)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto begin = std::find_if_not(value.begin(), value.end(), isSpace);
    auto end = std::find_if_not(value.rbegin(), value.rend(), isSpace).base();
    if (begin >= end) return {};
    return std::string(begin, end);
}

}

AppStateController::AppStateController(std::unique_ptr<IStorageManager> storageManager)
    : storageManager_(std::move(storageManager)) {
    if (storageManager_) {
        storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact){
            try {
                if (onDeletionImpact_) onDeletionImpact_(impact);
            } catch(...) {}
        });
    }
}

void AppStateController::notifyState() {
    try {
        if (onStateChanged_) onStateChanged_(state_);
    } catch (...) {}
}

void AppStateController::setStateChangedCallback(StateChanged cb) {
    onStateChanged_ = std::move(cb);
}

void AppStateController::setRepoFactory(IStorageManager::RepoFactory factory) {
    if (storageManager_) storageManager_->setRepoFactory(std::move(factory));
}

void AppStateController::setAtomicStoreSave(IStorageManager::AtomicStoreSave saveFn) {
    if (storageManager_) storageManager_->setAtomicStoreSave(std::move(saveFn));
}

void AppStateController::setAtomicStoreLoad(IStorageManager::AtomicStoreLoad loadFn) {
    if (storageManager_) storageManager_->setAtomicStoreLoad(std::move(loadFn));
}

void AppStateController::setDeletionImpactCallback(IStorageManager::DeletionImpactCallback cb) {
    onDeletionImpact_ = std::move(cb);
    if (storageManager_) storageManager_->setDeletionImpactCallback([this](const DeletionImpact& impact){
        try { if (onDeletionImpact_) onDeletionImpact_(impact); } catch(...) {}
    });
}

void AppStateController::openLatest() {
    if (!storageManager_) return;
    if (auto latest = storageManager_->loadLatestPath()) {
        try {
            fprintf(stderr, "AppStateController::openLatest: loading latest='%s'\n", latest->c_str());
        } catch(...) {}
        state_ = storageManager_->loadFrom(*latest);
        try {
            fprintf(stderr, "AppStateController::openLatest: loaded state - actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
                    state_.actors.size(), state_.properties.size(), state_.contracts.size(), state_.statements.size(), state_.transactions.size());
        } catch(...) {}
        notify();
    }
}

void AppStateController::newFile(const std::string& path) {
    if (!storageManager_) return;
    try {
        fprintf(stderr, "AppStateController::newFile: creating new file '%s' (will reset state)\n", path.c_str());
    } catch(...) {}
    storageManager_->createNew(path);
    state_ = AppState{};
    try {
        fprintf(stderr, "AppStateController::newFile: state reset -> actors=%zu props=%zu contracts=%zu statements=%zu transactions=%zu\n",
                state_.actors.size(), state_.properties.size(), state_.contracts.size(), state_.statements.size(), state_.transactions.size());
    } catch(...) {}
    notify();
}

void AppStateController::openFile(const std::string& path) {
    if (!storageManager_) return;
    state_ = storageManager_->loadFrom(path);
    notify();
}

void AppStateController::saveFile() {
    if (!storageManager_) return;
    storageManager_->save(state_);
}

void AppStateController::saveFileAs(const std::string& path) {
    if (!storageManager_) return;
    storageManager_->saveAs(path, state_);
}

std::string AppStateController::addActor(const std::string& name, const std::string& type, const std::string& description, const std::vector<std::string>& aliases)
{
    auto actor = std::make_shared<Actor>();
    actor->id = generateId();
    actor->name = name;
    actor->type = type;
    actor->description = description;
    actor->aliases = aliases;
    state_.actors.push_back(actor);
    commit();
    return actor->id;
}

void AppStateController::updateActor(const std::string& id, const std::string& name, const std::string& type, const std::string& description, const std::vector<std::string>& aliases)
{
    if (id.empty()) return;
    for (auto& actor : state_.actors) {
        if (!actor || actor->id != id) continue;
        actor->name = name;
        actor->type = type;
        actor->description = description;
        actor->aliases = aliases;
        commit();
        return;
    }
}

void AppStateController::deleteActor(const std::string& id)
{
    if (id.empty()) return;
    auto& actors = state_.actors;
    actors.erase(std::remove_if(actors.begin(), actors.end(), [&](const auto& actor) { return actor && actor->id == id; }), actors.end());
    commit();
}

std::string AppStateController::addProperty(const std::string& name, const std::string& address, const std::string& description)
{
    auto property = std::make_shared<Property>();
    property->id = generateId();
    property->name = name;
    property->address = address;
    property->description = description;
    state_.properties.push_back(property);
    commit();
    return property->id;
}

void AppStateController::updateProperty(const std::string& id, const std::string& name, const std::string& address, const std::string& description)
{
    if (id.empty()) return;
    for (auto& property : state_.properties) {
        if (!property || property->id != id) continue;
        property->name = name;
        property->address = address;
        property->description = description;
        commit();
        return;
    }
}

void AppStateController::deleteProperty(const std::string& id)
{
    if (id.empty()) return;
    auto& properties = state_.properties;
    properties.erase(std::remove_if(properties.begin(), properties.end(), [&](const auto& property) { return property && property->id == id; }), properties.end());
    commit();
}

std::string AppStateController::addContract(const std::string& name, const std::string& type, const std::string& description,
                                            const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds)
{
    auto contract = std::make_shared<Contract>();
    contract->id = generateId();
    contract->name = name;
    contract->type = type;
    contract->description = description;
    contract->actorIds = actorIds;
    contract->propertyIds = propertyIds;
    state_.contracts.push_back(contract);
    commit();
    return contract->id;
}

void AppStateController::updateContract(const std::string& id, const std::string& name, const std::string& type, const std::string& description,
                                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds)
{
    if (id.empty()) return;
    for (auto& contract : state_.contracts) {
        if (!contract || contract->id != id) continue;
        contract->name = name;
        contract->type = type;
        contract->description = description;
        contract->actorIds = actorIds;
        contract->propertyIds = propertyIds;
        commit();
        return;
    }
}

void AppStateController::deleteContract(const std::string& id)
{
    if (id.empty()) return;
    auto& contracts = state_.contracts;
    contracts.erase(std::remove_if(contracts.begin(), contracts.end(), [&](const auto& contract) { return contract && contract->id == id; }), contracts.end());
    commit();
}

std::string AppStateController::addStatement(const std::string& name)
{
    auto statement = std::make_shared<Statement>();
    statement->id = generateId();
    statement->name = name;
    state_.statements.push_back(statement);
    commit();
    return statement->id;
}

void AppStateController::updateStatement(const std::string& id, const std::string& name)
{
    if (id.empty()) return;
    for (auto& statement : state_.statements) {
        if (!statement || statement->id != id) continue;
        statement->name = name;
        commit();
        return;
    }
}

void AppStateController::deleteStatement(const std::string& id)
{
    if (id.empty()) return;
    std::vector<std::string> changedTxIds;
    auto& transactions = state_.transactions;
    transactions.erase(std::remove_if(transactions.begin(), transactions.end(), [&](const auto& tx) {
        if (!tx || tx->statementId != id) return false;
        changedTxIds.push_back(tx->id);
        return true;
    }), transactions.end());

    auto& statements = state_.statements;
    statements.erase(std::remove_if(statements.begin(), statements.end(), [&](const auto& statement) { return statement && statement->id == id; }), statements.end());
    commit();
}

std::string AppStateController::addTransaction(const std::string& name,
                                               const std::string& bookingDate,
                                               double amount,
                                               const std::string& description,
                                               const std::string& statementId,
                                               int status,
                                               const std::string& actorId,
                                               bool allocatable,
                                               const std::vector<std::string>& propertyIds)
{
    if (trimCopy(statementId).empty()) return {};
    auto tx = std::make_shared<Transaction>();
    tx->id = generateId();
    tx->name = name;
    tx->bookingDate = bookingDate;
    tx->amount = amount;
    tx->description = description;
    tx->statementId = statementId;
    tx->status = static_cast<Transaction::Status>(status);
    tx->actorId = actorId;
    tx->allocatable = allocatable;
    tx->propertyIds = propertyIds;
    tx->valuta.clear();
    tx->actorProposal.clear();
    tx->metadata.clear();
    tx->proofImagePath.clear();
    state_.transactions.push_back(tx);
    commit();
    return tx->id;
}

void AppStateController::updateTransaction(const std::string& id,
                                           const std::string& name,
                                           const std::string& bookingDate,
                                           double amount,
                                           const std::string& description,
                                           const std::string& statementId,
                                           int status,
                                           const std::string& actorId,
                                           bool allocatable,
                                           const std::vector<std::string>& propertyIds)
{
    if (id.empty()) return;
    for (auto& tx : state_.transactions) {
        if (!tx || tx->id != id) continue;
        tx->name = name;
        tx->bookingDate = bookingDate;
        tx->amount = amount;
        tx->description = description;
        tx->statementId = statementId;
        tx->status = static_cast<Transaction::Status>(status);
        tx->actorId = actorId;
        tx->allocatable = allocatable;
        tx->propertyIds = propertyIds;
        commit();
        return;
    }
}

void AppStateController::deleteTransaction(const std::string& id)
{
    if (id.empty()) return;
    std::vector<std::string> changedTxIds;
    auto& transactions = state_.transactions;
    transactions.erase(std::remove_if(transactions.begin(), transactions.end(), [&](const auto& tx) {
        if (!tx || tx->id != id) return false;
        changedTxIds.push_back(tx->id);
        return true;
    }), transactions.end());
    commit();
}

std::string AppStateController::addAnalysis(const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    auto analysis = std::make_shared<Analysis>();
    analysis->id = generateId();
    analysis->name = name;
    analysis->type = type;
    analysis->configJson = configJson;
    analysis->filterSpec = filterSpec;
    state_.analyses.push_back(analysis);
    commit();
    return analysis->id;
}

void AppStateController::updateAnalysis(const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec)
{
    if (id.empty()) return;
    for (auto& analysis : state_.analyses) {
        if (!analysis || analysis->id != id) continue;
        analysis->name = name;
        analysis->type = type;
        analysis->configJson = configJson;
        analysis->filterSpec = filterSpec;
        commit();
        return;
    }
}

void AppStateController::deleteAnalysis(const std::string& id)
{
    if (id.empty()) return;
    auto& analyses = state_.analyses;
    analyses.erase(std::remove_if(analyses.begin(), analyses.end(), [&](const auto& analysis) { return analysis && analysis->id == id; }), analyses.end());
    commit();
}

std::string AppStateController::addAnnual(int year)
{
    auto annual = std::make_shared<Annual>();
    annual->id = generateId();
    annual->year = year;
    state_.annuals.push_back(annual);
    commit();
    return annual->id;
}

void AppStateController::updateAnnual(const std::string& id, int year)
{
    if (id.empty()) return;
    for (auto& annual : state_.annuals) {
        if (!annual || annual->id != id) continue;
        annual->year = year;
        commit();
        return;
    }
}

void AppStateController::deleteAnnual(const std::string& id)
{
    if (id.empty()) return;
    auto& annuals = state_.annuals;
    annuals.erase(std::remove_if(annuals.begin(), annuals.end(), [&](const auto& annual) { return annual && annual->id == id; }), annuals.end());
    commit();
}

std::string AppStateController::finalizeStatementDraft(const DraftStatement& draft)
{
    if (draft.transactions.empty()) return {};

    auto statement = std::make_shared<Statement>();
    statement->id = generateId();
    statement->name = trimCopy(draft.name).empty() ? std::string("Imported") : draft.name;
    state_.statements.push_back(statement);

    std::vector<std::string> changedTxIds;
    changedTxIds.reserve(draft.transactions.size());

    for (const auto& item : draft.transactions) {
        auto tx = std::make_shared<Transaction>();
        tx->id = generateId();
        tx->name = item.name;
        tx->bookingDate = item.bookingDate;
        tx->amount = item.amount;
        tx->description = item.description;
        tx->statementId = statement->id;
        tx->status = static_cast<Transaction::Status>(item.status);
        tx->actorId = item.actorId;
        tx->allocatable = item.allocatable;
        tx->propertyIds = item.propertyIds;
        tx->valuta.clear();
        tx->actorProposal.clear();
        tx->metadata.clear();
        tx->proofImagePath.clear();

        const std::string normalizedType = trimCopy(item.type);
        if (!normalizedType.empty()) {
            int maxIdx = 0;
            constexpr const char* prefix = "Vertrag ";
            for (const auto& contractPtr : state_.contracts) {
                if (!contractPtr) continue;
                const std::string& contractName = contractPtr->name;
                if (contractName.size() <= std::char_traits<char>::length(prefix) || contractName.rfind(prefix, 0) != 0) continue;
                const std::string rest = contractName.substr(std::char_traits<char>::length(prefix));
                try {
                    const int idx = std::stoi(rest);
                    if (idx > maxIdx) maxIdx = idx;
                } catch (const std::exception&) {}
            }

            auto contract = std::make_shared<Contract>();
            contract->id = generateId();
            contract->name = std::string("Vertrag ") + std::to_string(maxIdx + 1);
            contract->type = normalizedType;
            contract->description.clear();
            contract->actorIds.clear();
            contract->propertyIds = item.propertyIds;
            state_.contracts.push_back(contract);
            tx->contractId = contract->id;
        }

        changedTxIds.push_back(tx->id);
        state_.transactions.push_back(tx);
    }

    commit();
    return statement->id;
}

void AppStateController::commit() {
    if (storageManager_) {
        try {
            const std::string& path = storageManager_->currentPath();
            if (!path.empty()) {
                storageManager_->save(state_);
            }
        } catch (const std::exception& ex) {
            fprintf(stderr, "AppStateController::commit: save failed: %s\n", ex.what());
        } catch (...) {
            fprintf(stderr, "AppStateController::commit: save failed: unknown error\n");
        }
    }

    try {
        if (onStateChanged_) onStateChanged_(state_);
    } catch (...) {}
}

void AppStateController::notify() {
    if (onStateChanged_) onStateChanged_(state_);
}
