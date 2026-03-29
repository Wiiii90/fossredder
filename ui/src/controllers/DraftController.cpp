/**
 * @file ui/src/controllers/DraftController.cpp
 * @brief Implements the UI controller that finalizes imported statement drafts.
 */

#include "ui/controllers/DraftController.h"

#include "core/application/AppStateFacade.h"
#include "core/import/DraftLinking.h"
#include "core/import/parsing/AmountParser.h"
#include "core/models/DraftStatement.h"
#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "ui/observability/Origins.h"

#include <QRegularExpression>
#include <QVariantList>

#include <vector>

namespace {

using core::importing::DraftChoiceRow;
using core::importing::DraftDerivedState;
using core::importing::DraftLinkSelection;
using core::importing::DraftSuggestionBucket;
using core::importing::DraftSuggestionCandidate;

const ui::TransactionDraft* currentDraft(ui::StatementDraft* draft)
{
    if (!draft || !draft->hasCurrent()) return nullptr;

    const auto index = draft->currentIndex();
    const auto& drafts = draft->transactions()->drafts();
    if (index < 0 || static_cast<std::size_t>(index) >= drafts.size()) return nullptr;
    return &drafts[static_cast<std::size_t>(index)];
}

DraftSuggestionCandidate toCoreCandidate(const ui::ImportSuggestion& suggestion)
{
    DraftSuggestionCandidate out;
    out.entityId = ui::strings::toStdString(suggestion.entityId);
    out.label = ui::strings::toStdString(suggestion.label);
    out.confidence = suggestion.confidence;
    return out;
}

DraftSuggestionBucket toCoreBucket(const ui::ImportSuggestionBucket& bucket)
{
    DraftSuggestionBucket out;
    out.candidates.reserve(bucket.candidates.size());
    for (const auto& suggestion : bucket.candidates) {
        out.candidates.push_back(toCoreCandidate(suggestion));
    }
    return out;
}

DraftLinkSelection toCoreSelection(const ui::TransactionDraft& draft)
{
    DraftLinkSelection out;
    out.name = ui::strings::toStdString(draft.name);
    out.description = ui::strings::toStdString(draft.description);
    out.metadata = ui::strings::toStdString(draft.metadata);
    out.proofImagePath = ui::strings::toStdString(draft.proofImagePath);
    out.actorText = ui::strings::toStdString(draft.actorText);
    out.propertyText = ui::strings::toStdString(draft.propertyText);
    out.actorId = ui::strings::toStdString(draft.actorId);
    out.newActorSelected = draft.newActorSelected;
    out.contractId = ui::strings::toStdString(draft.contractId);
    out.newContractSelected = draft.newContractSelected;
    out.type = ui::strings::toStdString(draft.type);
    out.allocatable = draft.allocatable;
    out.allocatableManualOverride = draft.allocatableManualOverride;
    out.propertyIds = ui::strings::toStdList(draft.propertyIds);
    out.actorSuggestions = toCoreBucket(draft.suggestions.actor);
    out.propertySuggestions = toCoreBucket(draft.suggestions.property);
    out.contractSuggestions = toCoreBucket(draft.suggestions.contract);
    return out;
}

QStringList toQStringList(const std::vector<std::string>& values)
{
    QStringList out;
    out.reserve(static_cast<int>(values.size()));
    for (const auto& value : values) {
        out.push_back(QString::fromStdString(value));
    }
    return out;
}

QVariantMap toVariantMap(const DraftSuggestionCandidate& suggestion)
{
    QVariantMap map;
    map.insert(QStringLiteral("entityId"), QString::fromStdString(suggestion.entityId));
    map.insert(QStringLiteral("label"), QString::fromStdString(suggestion.label));
    map.insert(QStringLiteral("confidence"), suggestion.confidence);
    return map;
}

const DraftChoiceRow* findChoiceRowById(const std::vector<DraftChoiceRow>& rows, const std::string& id)
{
    if (id.empty()) return nullptr;
    for (const auto& row : rows) {
        if (row.id == id) return &row;
    }
    return nullptr;
}

QVariantMap toVariantMap(const DraftChoiceRow& row)
{
    QVariantMap map;
    map.insert(QStringLiteral("id"), QString::fromStdString(row.id));
    map.insert(QStringLiteral("name"), QString::fromStdString(row.name));
    map.insert(QStringLiteral("display"), QString::fromStdString(row.display));
    map.insert(QStringLiteral("type"), QString::fromStdString(row.type));
    map.insert(QStringLiteral("aliases"), toQStringList(row.aliases));
    map.insert(QStringLiteral("actorIds"), toQStringList(row.actorIds));
    map.insert(QStringLiteral("propertyIds"), toQStringList(row.propertyIds));
    map.insert(QStringLiteral("synthetic"), row.synthetic);
    map.insert(QStringLiteral("confidence"), row.confidence);
    map.insert(QStringLiteral("sourceText"), QString::fromStdString(row.sourceText));
    return map;
}

QVariantList toVariantList(const std::vector<DraftChoiceRow>& rows)
{
    QVariantList out;
    out.reserve(static_cast<int>(rows.size()));
    for (const auto& row : rows) {
        out.push_back(toVariantMap(row));
    }
    return out;
}

QVariantMap toViewState(const DraftDerivedState& derived)
{
    QVariantMap map;
    map.insert(QStringLiteral("proofSource"), QString::fromStdString(derived.proofSource));
    map.insert(QStringLiteral("actorSeedText"), QString::fromStdString(derived.actorSeedText));
    map.insert(QStringLiteral("actorDisplayText"), QString::fromStdString(derived.actorDisplayText));
    map.insert(QStringLiteral("contractSeedText"), QString::fromStdString(derived.contractSeedText));
    map.insert(QStringLiteral("contractDisplayText"), QString::fromStdString(derived.contractDisplayText));
    map.insert(QStringLiteral("propertySuggestionSummary"), QString::fromStdString(derived.propertySuggestionSummary));
    map.insert(QStringLiteral("effectiveAllocatable"), derived.effectiveAllocatable);
    map.insert(QStringLiteral("actorCurrentIndex"), derived.actorCurrentIndex);
    map.insert(QStringLiteral("contractCurrentIndex"), derived.contractCurrentIndex);
    map.insert(QStringLiteral("actorTopSuggestion"), derived.hasActorTopSuggestion ? toVariantMap(derived.actorTopSuggestion) : QVariantMap{});
    map.insert(QStringLiteral("propertyTopSuggestion"), derived.hasPropertyTopSuggestion ? toVariantMap(derived.propertyTopSuggestion) : QVariantMap{});
    map.insert(QStringLiteral("contractTopSuggestion"), derived.hasContractTopSuggestion ? toVariantMap(derived.contractTopSuggestion) : QVariantMap{});
    map.insert(QStringLiteral("actorChoices"), toVariantList(derived.actorChoices));
    map.insert(QStringLiteral("contractChoices"), toVariantList(derived.contractChoices));
    map.insert(QStringLiteral("propertyRows"), toVariantList(derived.propertyRows));
    map.insert(QStringLiteral("autoPropertyIds"), toQStringList(derived.autoPropertyIds));
    return map;
}

QString rowDisplayText(const QVariantMap& row)
{
    const QString display = row.value(QStringLiteral("display")).toString();
    if (!display.isEmpty()) return display;
    const QString name = row.value(QStringLiteral("name")).toString();
    if (!name.isEmpty()) return name;
    const QString type = row.value(QStringLiteral("type")).toString();
    if (!type.isEmpty()) return type;
    return row.value(QStringLiteral("id")).toString();
}

bool rowMatchesText(const QVariantMap& row, const QString& text)
{
    const auto match = [&](const QString& value) {
        return core::importing::matchesDraftText(ui::strings::toStdString(value), ui::strings::toStdString(text));
    };

    if (match(row.value(QStringLiteral("name")).toString())) return true;
    if (match(row.value(QStringLiteral("display")).toString())) return true;
    if (match(row.value(QStringLiteral("type")).toString())) return true;

    for (const auto& alias : row.value(QStringLiteral("aliases")).toStringList()) {
        if (match(alias)) return true;
    }
    return false;
}

core::domain::AppState matchingStateForDraft(const ui::StatementDraft* draft,
                                             const core::application::AppStateFacade* core)
{
    const auto liveState = core ? core->state() : core::domain::AppState{};
    if (!draft || !draft->hasCatalogState()) return liveState;

    auto state = draft->catalogState();
    if (state.actors.empty()) state.actors = liveState.actors;
    if (state.properties.empty()) state.properties = liveState.properties;
    if (state.contracts.empty()) state.contracts = liveState.contracts;
    if (state.statements.empty()) state.statements = liveState.statements;
    if (state.transactions.empty()) state.transactions = liveState.transactions;
    if (state.analyses.empty()) state.analyses = liveState.analyses;
    if (state.annuals.empty()) state.annuals = liveState.annuals;
    return state;
}

} // namespace

namespace ui {

DraftController::DraftController(core::application::AppStateFacade* core, QObject* parent)
    : QObject(parent), core_(core)
{
}

QString DraftController::finalizeStatementDraft(StatementDraft* draft)
{
    if (!draft) return {};

    return controllers::guard::invokeValue<QString>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto& drafts = draft->transactions()->drafts();
            if (drafts.empty()) return QString{};
            const auto appState = matchingStateForDraft(draft, core_);

            DraftStatement input;
            input.name = strings::toStdString(draft->name());
            input.transactions.reserve(drafts.size());

            for (const auto& d : drafts) {
                DraftTransaction transaction;
                transaction.name = strings::toStdString(d.name);
                transaction.bookingDate = strings::toStdString(d.bookingDate);
                transaction.amount = d.amount;
                transaction.description = strings::toStdString(d.description);
                transaction.status = static_cast<Transaction::Status>(d.status);

                QString actorId = d.actorId;
                if (actorId.isEmpty() && (!d.actorText.trimmed().isEmpty() || d.newActorSelected)) {
                    actorId = QString::fromStdString(core::importing::resolveActorId(appState, strings::toStdString(d.actorText)));
                    if (actorId.isEmpty()) {
                        const QString actorName = d.actorText.trimmed();
                        if (!actorName.isEmpty()) {
                            const std::vector<std::string> aliases{strings::toStdString(actorName)};
                            actorId = QString::fromStdString(core_->addActor(strings::toStdString(actorName), std::string{}, std::string{}, aliases));
                        }
                    }
                }

                QString contractId = d.contractId;
                if (contractId.isEmpty() && d.newContractSelected) {
                    const QString contractType = d.type.trimmed();
                    if (!contractType.isEmpty()) {
                        std::vector<std::string> actorIds;
                        if (!actorId.isEmpty()) actorIds.push_back(strings::toStdString(actorId));
                        std::vector<std::string> propertyIds = strings::toStdList(d.propertyIds);
                        std::vector<std::string> aliases = core::importing::referenceAliasesFromMetadata(strings::toStdString(d.metadata));
                        contractId = QString::fromStdString(core_->addContract(std::string{}, strings::toStdString(contractType), std::string{}, actorIds, propertyIds, aliases));
                    }
                }

                transaction.actorId = strings::toStdString(actorId);
                transaction.contractId = strings::toStdString(contractId);
                transaction.allocatable = d.allocatableManualOverride
                                           ? d.allocatable
                                           : (core::importing::contractIsFullyAllocatable(appState, strings::toStdString(contractId)) || d.allocatable);
                transaction.propertyIds = strings::toStdList(d.propertyIds);
                transaction.type = strings::toStdString(d.type);
                input.transactions.push_back(std::move(transaction));
            }

            return QString::fromStdString(core_->finalizeStatementDraft(input));
        });
}

QVariantMap DraftController::currentTransactionViewState(StatementDraft* draft) const
{
    return controllers::guard::invokeValue<QVariantMap>(
        core_, observability::origins::controller::draft::kFinalize, {}, [&]() {
            const auto* current = currentDraft(draft);
            if (!current) return QVariantMap{};
            return toViewState(core::importing::buildDraftDerivedState(matchingStateForDraft(draft, core_), toCoreSelection(*current)));
        });
}

QVariantMap DraftController::findChoiceRowByText(const QVariantList& rows, const QString& text) const
{
    for (const auto& item : rows) {
        const QVariantMap row = item.toMap();
        if (!row.isEmpty() && rowMatchesText(row, text)) return row;
    }
    return {};
}

void DraftController::syncCurrentTransactionDraft(StatementDraft* draft)
{
    controllers::guard::invokeVoid(core_, observability::origins::controller::draft::kFinalize, [&]() {
        const auto* current = currentDraft(draft);
        if (!draft || !current) return;

        const auto appState = matchingStateForDraft(draft, core_);
        const auto derived = core::importing::buildDraftDerivedState(appState, toCoreSelection(*current));
        const auto index = draft->currentIndex();
        bool changed = false;

        if (current->actorId.isEmpty()) {
            if (derived.actorCurrentIndex > 0 && static_cast<std::size_t>(derived.actorCurrentIndex) < derived.actorChoices.size()) {
                const auto& actorRow = derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
                if (!actorRow.synthetic && !actorRow.id.empty()) {
                    const QString actorId = QString::fromStdString(actorRow.id);
                    const QString actorText = QString::fromStdString(actorRow.display.empty() ? actorRow.name : actorRow.display);
                    changed = changed || current->actorId != actorId || current->actorText != actorText || current->newActorSelected;
                    draft->transactions()->setActorId(index, QString::fromStdString(actorRow.id));
                    draft->transactions()->setActorText(index, actorText);
                    draft->transactions()->setNewActorSelected(index, false);
                }
            } else if (!derived.actorSeedText.empty()) {
                const QString actorText = QString::fromStdString(derived.actorSeedText);
                changed = changed || current->actorText != actorText;
                draft->transactions()->setActorText(index, actorText);
            }
        }

        if (current->contractId.isEmpty()) {
            if (derived.contractCurrentIndex > 0 && static_cast<std::size_t>(derived.contractCurrentIndex) < derived.contractChoices.size()) {
                const auto& contractRow = derived.contractChoices[static_cast<std::size_t>(derived.contractCurrentIndex)];
                if (!contractRow.synthetic && !contractRow.id.empty()) {
                    const QString contractId = QString::fromStdString(contractRow.id);
                    changed = changed || current->contractId != contractId || current->newContractSelected;
                    draft->transactions()->setContractId(index, contractId);
                    if (!contractRow.type.empty()) {
                        const QString type = QString::fromStdString(contractRow.type);
                        changed = changed || current->type != type;
                        draft->transactions()->setType(index, type);
                    }
                    if (!contractRow.actorIds.empty()) {
                        const QString actorId = QString::fromStdString(contractRow.actorIds.front());
                        changed = changed || current->actorId != actorId || current->newActorSelected;
                        draft->transactions()->setActorId(index, actorId);
                        if (const auto* actorRow = findChoiceRowById(derived.actorChoices, contractRow.actorIds.front())) {
                            const QString actorText = QString::fromStdString(actorRow->display.empty() ? actorRow->name : actorRow->display);
                            changed = changed || current->actorText != actorText;
                            draft->transactions()->setActorText(index, actorText);
                        }
                        draft->transactions()->setNewActorSelected(index, false);
                    }
                    if (!contractRow.propertyIds.empty()) {
                        const auto propertyIds = toQStringList(contractRow.propertyIds);
                        changed = changed || current->propertyIds != propertyIds;
                        draft->transactions()->setProperties(index, propertyIds);
                    }
                    draft->transactions()->setNewContractSelected(index, false);
                }
            } else if (!derived.contractSeedText.empty()) {
                const QString type = QString::fromStdString(derived.contractSeedText);
                changed = changed || current->type != type;
                draft->transactions()->setType(index, type);
            } else if (!current->newContractSelected) {
                changed = changed || !current->type.isEmpty();
                draft->transactions()->setType(index, QString());
            }
        }

        if (current->propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
            const auto propertyIds = toQStringList(derived.autoPropertyIds);
            changed = changed || current->propertyIds != propertyIds;
            draft->transactions()->setProperties(index, propertyIds);
        }

        if (!current->allocatableManualOverride) {
            changed = changed || current->allocatable != derived.effectiveAllocatable;
            draft->transactions()->setAllocatable(index, derived.effectiveAllocatable);
        }

        if (changed) draft->refresh();
    });
}

void DraftController::selectCurrentActorChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setActorId(draft->currentIndex(), QString());
        draft->transactions()->setActorText(draft->currentIndex(), QString());
        draft->transactions()->setNewActorSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setActorId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        draft->transactions()->setActorText(draft->currentIndex(), rowDisplayText(row));
        draft->transactions()->setNewActorSelected(draft->currentIndex(), false);
    }
}

void DraftController::selectCurrentContractChoice(StatementDraft* draft, const QVariantMap& row)
{
    if (!draft || row.isEmpty()) return;

    if (row.value(QStringLiteral("synthetic")).toBool()) {
        draft->transactions()->setContractId(draft->currentIndex(), QString());
        draft->transactions()->setType(draft->currentIndex(), QString());
        draft->transactions()->setNewContractSelected(draft->currentIndex(), true);
    } else {
        draft->transactions()->setContractId(draft->currentIndex(), row.value(QStringLiteral("id")).toString());
        const QString type = row.value(QStringLiteral("type")).toString();
        if (!type.isEmpty()) draft->transactions()->setType(draft->currentIndex(), type);

        const auto actorIds = row.value(QStringLiteral("actorIds")).toStringList();
        if (!actorIds.isEmpty()) draft->transactions()->setActorId(draft->currentIndex(), actorIds.first());

        const auto propertyIds = row.value(QStringLiteral("propertyIds")).toStringList();
        if (!propertyIds.isEmpty()) draft->transactions()->setProperties(draft->currentIndex(), propertyIds);

        draft->transactions()->setNewContractSelected(draft->currentIndex(), false);
    }
}

void DraftController::setCurrentPropertySelected(StatementDraft* draft, const QString& propertyId, bool selected)
{
    const auto* current = currentDraft(draft);
    if (!draft || !current || propertyId.isEmpty()) return;

    QStringList propertyIds = current->propertyIds;
    const int index = propertyIds.indexOf(propertyId);
    if (selected) {
        if (index < 0) propertyIds.push_back(propertyId);
    } else if (index >= 0) {
        propertyIds.removeAt(index);
    }

    draft->transactions()->setProperties(draft->currentIndex(), propertyIds);
}

void DraftController::updateCurrentAmount(StatementDraft* draft, const QString& text)
{
    if (!draft) return;

    const auto parsed = core::parser::parseAmountString(text.trimmed().toStdString());
    if (!parsed) {
        draft->transactions()->setAmount(draft->currentIndex(), 0.0);
        return;
    }

    draft->transactions()->setAmount(draft->currentIndex(), *parsed);
}

} // namespace ui
