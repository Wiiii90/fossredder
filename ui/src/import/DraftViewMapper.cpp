/**
 * @file ui/src/import/DraftViewMapper.cpp
 * @brief Implements helpers that map draft suggestions and derived state into UI-friendly values.
 */

#include "ui/import/DraftViewMapper.h"

#include "ui/models/TransactionDraft.h"
#include "ui/payload/PayloadMapper.h"
#include "ui/util/StringConversions.h"

namespace core_importing = core::application::importing;

namespace ui::importing {

core_importing::DraftSuggestionCandidate toCoreCandidate(const ImportSuggestion& suggestion)
{
    core_importing::DraftSuggestionCandidate out;
    out.entityId = ui::strings::toStdString(suggestion.entityId);
    out.label = ui::strings::toStdString(suggestion.label);
    out.confidence = suggestion.confidence;
    return out;
}

core_importing::DraftSuggestionBucket toCoreBucket(const ImportSuggestionBucket& bucket)
{
    core_importing::DraftSuggestionBucket out;
    out.candidates.reserve(bucket.candidates.size());
    for (const auto& suggestion : bucket.candidates) {
        out.candidates.push_back(toCoreCandidate(suggestion));
    }
    return out;
}

core_importing::DraftLinkSelection toCoreSelection(const TransactionDraft& draft)
{
    core_importing::DraftLinkSelection out;
    out.name = ui::strings::toStdString(draft.name);
    out.metadata = ui::strings::toStdString(draft.metadata);
    out.actorText = ui::strings::toStdString(draft.actorText);
    out.propertyText = ui::strings::toStdString(draft.propertyText);
    out.actorId = ui::strings::toStdString(draft.actorId);
    out.actorSelected = draft.actorSelected;
    out.contractId = ui::strings::toStdString(draft.contractId);
    out.contractSelected = draft.contractSelected;
    out.type = ui::strings::toStdString(draft.type);
    out.allocatable = draft.allocatable;
    out.allocatableSelected = draft.allocatableSelected;
    out.propertyIds = ui::strings::toStdList(draft.propertyIds);
    out.actorSuggestions = toCoreBucket(draft.suggestions.actor);
    out.propertySuggestions = toCoreBucket(draft.suggestions.property);
    out.contractSuggestions = toCoreBucket(draft.suggestions.contract);
    return out;
}

QStringList toQStringList(const std::vector<std::string>& values)
{
    return payload::mapper::toQStringList(values);
}

QVariantMap toVariantMap(const core_importing::DraftSuggestionCandidate& suggestion)
{
    QVariantMap map;
    map.insert(QStringLiteral("entityId"), QString::fromStdString(suggestion.entityId));
    map.insert(QStringLiteral("label"), QString::fromStdString(suggestion.label));
    map.insert(QStringLiteral("confidence"), suggestion.confidence);
    return map;
}

QVariantMap toVariantMap(const core_importing::DraftChoiceRow& row)
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

QVariantList toVariantList(const std::vector<core_importing::DraftChoiceRow>& rows)
{
    QVariantList out;
    out.reserve(static_cast<int>(rows.size()));
    for (const auto& row : rows) {
        out.push_back(toVariantMap(row));
    }
    return out;
}

QVariantMap toViewState(const core_importing::DraftDerivedState& derived)
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
    if (!display.isEmpty()) {
        return display;
    }

    const QString name = row.value(QStringLiteral("name")).toString();
    if (!name.isEmpty()) {
        return name;
    }

    const QString type = row.value(QStringLiteral("type")).toString();
    if (!type.isEmpty()) {
        return type;
    }

    return row.value(QStringLiteral("id")).toString();
}

QString choiceDisplayText(const core_importing::DraftChoiceRow& row)
{
    if (!row.display.empty()) {
        return QString::fromStdString(row.display);
    }
    if (!row.name.empty()) {
        return QString::fromStdString(row.name);
    }
    if (!row.type.empty()) {
        return QString::fromStdString(row.type);
    }
    return QString::fromStdString(row.id);
}

bool rowMatchesText(const QVariantMap& row, const QString& text)
{
    const auto normalizedText = text.trimmed().simplified();
    if (normalizedText.isEmpty()) {
        return false;
    }

    const auto match = [&](const QString& value) {
        return value.trimmed().simplified().compare(normalizedText, Qt::CaseInsensitive) == 0;
    };

    if (match(row.value(QStringLiteral("name")).toString())) {
        return true;
    }
    if (match(row.value(QStringLiteral("display")).toString())) {
        return true;
    }
    if (match(row.value(QStringLiteral("type")).toString())) {
        return true;
    }

    for (const auto& alias : row.value(QStringLiteral("aliases")).toStringList()) {
        if (match(alias)) {
            return true;
        }
    }
    return false;
}

const core_importing::DraftChoiceRow* findChoiceRowById(const std::vector<core_importing::DraftChoiceRow>& rows,
                                                        const std::string& id)
{
    if (id.empty()) {
        return nullptr;
    }

    for (const auto& row : rows) {
        if (row.id == id) {
            return &row;
        }
    }
    return nullptr;
}

} // namespace ui::importing

