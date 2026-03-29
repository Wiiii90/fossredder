/**
 * @file ui/include/ui/controllers/DraftProjection.h
 * @brief Declares helper functions that project draft UI state into core-friendly data.
 */

#pragma once

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <string>
#include <vector>

#include "core/import/DraftLinking.h"
#include "core/models/AppState.h"
#include "core/models/DraftStatement.h"
#include "ui/models/ImportSuggestion.h"

namespace core::application { class AppStateFacade; }
namespace ui { class StatementDraft; class TransactionDraft; }

namespace ui {

using core::importing::DraftChoiceRow;
using core::importing::DraftDerivedState;
using core::importing::DraftLinkSelection;
using core::importing::DraftSuggestionBucket;
using core::importing::DraftSuggestionCandidate;

const TransactionDraft* currentDraft(StatementDraft* draft);
DraftSuggestionCandidate toCoreCandidate(const ImportSuggestion& suggestion);
DraftSuggestionBucket toCoreBucket(const ImportSuggestionBucket& bucket);
DraftLinkSelection toCoreSelection(const TransactionDraft& draft);
QStringList toQStringList(const std::vector<std::string>& values);
QVariantMap toVariantMap(const DraftSuggestionCandidate& suggestion);
QVariantMap toVariantMap(const DraftChoiceRow& row);
QVariantList toVariantList(const std::vector<DraftChoiceRow>& rows);
QVariantMap toViewState(const DraftDerivedState& derived);
QString rowDisplayText(const QVariantMap& row);
bool rowMatchesText(const QVariantMap& row, const QString& text);
const DraftChoiceRow* findChoiceRowById(const std::vector<DraftChoiceRow>& rows, const std::string& id);
core::domain::AppState matchingStateForDraft(const StatementDraft* draft,
                                             const core::application::AppStateFacade* core);
core::domain::DraftStatement buildFinalizationInput(StatementDraft* draft,
                                                    core::application::AppStateFacade* core);
void syncCurrentTransactionDraft(StatementDraft* draft,
                                 core::application::AppStateFacade* core);

} // namespace ui
