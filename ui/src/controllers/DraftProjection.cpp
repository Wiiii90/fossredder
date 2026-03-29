/**
 * @file ui/src/controllers/DraftProjection.cpp
 * @brief Implements helper functions that project draft UI state into core-friendly data.
 */

#include "ui/controllers/DraftProjection.h"

#include "core/application/AppStateFacade.h"
#include "ui/models/StatementDraft.h"
#include "ui/models/TransactionDraft.h"
#include "ui/controllers/ControllerStrings.h"

#include <QVariantList>
#include <QVariantMap>

#include <utility>

namespace ui {

const TransactionDraft* currentDraft(StatementDraft* draft)
{
    if (!draft || !draft->hasCurrent()) return nullptr;

    const auto index = draft->currentIndex();
    const auto& drafts = draft->transactions()->drafts();
    if (index < 0 || static_cast<std::size_t>(index) >= drafts.size()) return nullptr;
    return &drafts[static_cast<std::size_t>(index)];
}

DraftSuggestionCandidate toCoreCandidate(const ImportSuggestion& suggestion)
{
    DraftSuggestionCandidate out;
    out.entityId = ui::strings::toStdString(suggestion.entityId);
    out.label = ui::strings::toStdString(suggestion.label);
    out.confidence = suggestion.confidence;
    return out;
}

DraftSuggestionBucket toCoreBucket(const ImportSuggestionBucket& bucket)
{
    DraftSuggestionBucket out;
    out.candidates.reserve(bucket.candidates.size());
    for (const auto& suggestion : bucket.candidates) {
        out.candidates.push_back(toCoreCandidate(suggestion));
    }
    return out;
}

DraftLinkSelection toCoreSelection(const TransactionDraft& draft)
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

const DraftChoiceRow* findChoiceRowById(const std::vector<DraftChoiceRow>& rows, const std::string& id)
{
    if (id.empty()) return nullptr;
    for (const auto& row : rows) {
        if (row.id == id) return &row;
    }
    return nullptr;
}

core::domain::AppState matchingStateForDraft(const StatementDraft* draft,
                                             const core::application::AppStateFacade* core)
{
    const auto liveState = core ? core->state() : core::domain::AppState{};
    if (!draft) return liveState;

    return core::importing::withFallbackState(
        draft->hasCatalogState() ? draft->catalogState() : core::domain::AppState{},
        liveState);
}

} // namespace ui
