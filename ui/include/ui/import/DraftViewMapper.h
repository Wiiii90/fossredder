/**
 * @file ui/include/ui/import/DraftViewMapper.h
 * @brief Declares helpers that map draft suggestions and derived state into UI-friendly values.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <string>
#include <vector>

#include "core/application/import/draft/DraftMatcher.h"
#include "ui/models/ImportSuggestion.h"

namespace ui {
struct TransactionDraft;
}

namespace core_importing = core::application::importing;

namespace ui::importing {

/** @brief Convert a UI suggestion candidate into the corresponding core draft candidate. */
core_importing::DraftSuggestionCandidate toCoreCandidate(const ImportSuggestion& suggestion);

/** @brief Convert a UI suggestion bucket into the corresponding core draft bucket. */
core_importing::DraftSuggestionBucket toCoreBucket(const ImportSuggestionBucket& bucket);

/** @brief Convert a transaction draft into the corresponding core link selection. */
core_importing::DraftLinkSelection toCoreSelection(const TransactionDraft& draft);

/** @brief Convert a list of standard strings into a Qt string list. */
QStringList toQStringList(const std::vector<std::string>& values);

/** @brief Convert a draft suggestion candidate into a QML-friendly map. */
QVariantMap toVariantMap(const core_importing::DraftSuggestionCandidate& suggestion);

/** @brief Convert a draft choice row into a QML-friendly map. */
QVariantMap toVariantMap(const core_importing::DraftChoiceRow& row);

/** @brief Convert a list of draft choice rows into a QML-friendly list. */
QVariantList toVariantList(const std::vector<core_importing::DraftChoiceRow>& rows);

/** @brief Convert derived draft state into a QML-friendly view-state map. */
QVariantMap toViewState(const core_importing::DraftDerivedState& derived);

/** @brief Resolve the best display text for a serialized draft choice row. */
QString rowDisplayText(const QVariantMap& row);

/** @brief Resolve the best display text for a core draft choice row. */
QString choiceDisplayText(const core_importing::DraftChoiceRow& row);

/** @brief Check whether a serialized draft choice row matches the provided search text. */
bool rowMatchesText(const QVariantMap& row, const QString& text);

/** @brief Find a draft choice row by identifier. */
const core_importing::DraftChoiceRow* findChoiceRowById(const std::vector<core_importing::DraftChoiceRow>& rows,
                                                        const std::string& id);

} // namespace ui::importing
