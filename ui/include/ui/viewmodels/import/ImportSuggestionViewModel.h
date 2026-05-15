/**
 * @file ui/include/ui/viewmodels/import/ImportSuggestionViewModel.h
 * @brief UI-side models for ranked import matching suggestions.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <vector>

namespace ui {

/**
 * @brief Describes a single ranked suggestion for an imported draft field.
 */
struct ImportSuggestion {
    QString entityId;
    QString entityType;
    QString label;
    QString sourceText;
    QString rationale;
    QString lastUsedAt;
    QStringList matchedAliases;
    double score = 0.0;
    double confidence = 0.0;
    double aliasWeight = 0.0;
    double recencyWeight = 0.0;
    int hitCount = 0;
};

/**
 * @brief Groups a suggestion source with ranked candidates.
 */
struct ImportSuggestionBucket {
    QString sourceText;
    std::vector<ImportSuggestion> candidates;
};

/**
 * @brief Holds suggestions for the main import draft entity links.
 */
struct ImportDraftSuggestions {
    ImportSuggestionBucket actor;
    ImportSuggestionBucket property;
    ImportSuggestionBucket contract;
};

inline QVariantMap toVariantMap(const ImportSuggestion& suggestion)
{
    QVariantMap map;
    map.insert(QStringLiteral("entityId"), suggestion.entityId);
    map.insert(QStringLiteral("entityType"), suggestion.entityType);
    map.insert(QStringLiteral("label"), suggestion.label);
    map.insert(QStringLiteral("sourceText"), suggestion.sourceText);
    map.insert(QStringLiteral("rationale"), suggestion.rationale);
    map.insert(QStringLiteral("lastUsedAt"), suggestion.lastUsedAt);
    map.insert(QStringLiteral("matchedAliases"), suggestion.matchedAliases);
    map.insert(QStringLiteral("score"), suggestion.score);
    map.insert(QStringLiteral("confidence"), suggestion.confidence);
    map.insert(QStringLiteral("aliasWeight"), suggestion.aliasWeight);
    map.insert(QStringLiteral("recencyWeight"), suggestion.recencyWeight);
    map.insert(QStringLiteral("hitCount"), suggestion.hitCount);
    return map;
}

inline QVariantMap toVariantMap(const ImportSuggestionBucket& bucket)
{
    QVariantMap map;
    map.insert(QStringLiteral("sourceText"), bucket.sourceText);

    QVariantList candidates;
    candidates.reserve(static_cast<int>(bucket.candidates.size()));
    for (const auto& suggestion : bucket.candidates) {
        candidates.push_back(toVariantMap(suggestion));
    }
    map.insert(QStringLiteral("candidates"), candidates);
    return map;
}

inline QVariantMap toVariantMap(const ImportDraftSuggestions& suggestions)
{
    QVariantMap map;
    map.insert(QStringLiteral("actor"), toVariantMap(suggestions.actor));
    map.insert(QStringLiteral("property"), toVariantMap(suggestions.property));
    map.insert(QStringLiteral("contract"), toVariantMap(suggestions.contract));
    return map;
}

} // namespace ui

