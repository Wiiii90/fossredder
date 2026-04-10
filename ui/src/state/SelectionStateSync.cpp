/**
 * @file ui/src/state/SelectionStateSync.cpp
 * @brief Implements helpers for selection refresh tracking.
 */

#include "ui/state/SelectionStateSync.h"

#include "ui/state/SessionSelection.h"

namespace ui {

SelectionIdsSnapshot captureSelectionIds(const SelectionState& selection)
{
    return {
        selection.selectedActorId(),
        selection.selectedPropertyId(),
        selection.selectedContractId(),
        selection.selectedStatementId(),
        selection.selectedTransactionId(),
        selection.selectedAnalysisId(),
        selection.selectedAnnualId()
    };
}

void emitSelectionChanges(SessionSelection& selection, const SelectionIdsSnapshot& before)
{
    if (selection.selectedActorId() != before.actorId) emit selection.selectedActorIdChanged();
    if (selection.selectedPropertyId() != before.propertyId) emit selection.selectedPropertyIdChanged();
    if (selection.selectedContractId() != before.contractId) emit selection.selectedContractIdChanged();
    if (selection.selectedStatementId() != before.statementId) emit selection.selectedStatementIdChanged();
    if (selection.selectedTransactionId() != before.transactionId) emit selection.selectedTransactionIdChanged();
    if (selection.selectedAnalysisId() != before.analysisId) emit selection.selectedAnalysisIdChanged();
    if (selection.selectedAnnualId() != before.annualId) emit selection.selectedAnnualIdChanged();
}

} // namespace ui
