/**
 * @file ui/src/state/import/StatementDraftState.cpp
 * @brief Implements statement draft review state.
 */

#include "ui/state/import/StatementDraftState.h"

#include "ui/state/import/TransactionDraftState.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"

namespace ui {

StatementDraftState::StatementDraftState(QObject *parent) : QObject(parent) {}

void StatementDraftState::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  bindImportWorkflow(value);
  emit changed();
}

void StatementDraftState::setNavigation(NavigationState *value) {
  if (navigation_ == value) {
    return;
  }
  navigation_ = value;
  emit changed();
}

void StatementDraftState::setDraft(StatementDraft *value) {
  if (draft_ == value) {
    return;
  }
  bindDraft(value);
  emit changed();
}

void StatementDraftState::setTransactionState(TransactionDraftState *value) {
  if (transactionState_ == value) {
    return;
  }
  transactionState_ = value;
  emit changed();
}

bool StatementDraftState::hasDraft() const noexcept { return draft_ != nullptr; }

QString StatementDraftState::statementName() const {
  return draft_ ? draft_->name() : QString();
}

void StatementDraftState::setStatementName(const QString &value) {
  if (!draft_ || draft_->name() == value) {
    return;
  }
  draft_->setName(value);
  emit changed();
}

QString StatementDraftState::transactionInfoText() const {
  if (!draft_) {
    return tr("No current transaction");
  }
  return tr("Transaction %1 / %2").arg(draft_->currentIndex() + 1).arg(
      draft_->count());
}

bool StatementDraftState::canAddTransaction() const noexcept {
  return draft_ != nullptr;
}

bool StatementDraftState::canDeleteTransaction() const noexcept {
  return draft_ && draft_->count() > 1;
}

bool StatementDraftState::canOpenPreviousTransaction() const noexcept {
  return draft_ && draft_->currentIndex() > 0;
}

bool StatementDraftState::canOpenNextTransaction() const noexcept {
  return draft_ && draft_->currentIndex() < draft_->count() - 1;
}

bool StatementDraftState::canOpenPreviousDraft() const noexcept {
  return draft_ && importWorkflow_ && importWorkflow_->hasPrevDraft();
}

bool StatementDraftState::canOpenNextDraft() const noexcept {
  return draft_ && importWorkflow_ && importWorkflow_->hasNextDraft();
}

void StatementDraftState::commitPendingEdits() {
  if (transactionState_) {
    transactionState_->commitPendingEdits();
  }
}

void StatementDraftState::persistPendingEdits() {
  commitPendingEdits();
  if (importWorkflow_ && draft_) {
    importWorkflow_->persistStatementDraft(draft_);
  }
}

void StatementDraftState::returnToImport() {
  persistPendingEdits();
  const QString draftId = currentDraftId();
  if (importWorkflow_) {
    importWorkflow_->addRunNote(
        tr("Draft"), tr("Draft paused. Click log entry to continue."), true,
        QString(), draftId);
    importWorkflow_->clearDraft();
  }
  navigateToImport();
}

void StatementDraftState::discardDraft() {
  const QString draftId = currentDraftId();
  if (importWorkflow_) {
    importWorkflow_->clearPersistedStatementDraft(draftId);
    importWorkflow_->addRunNote(tr("Draft discarded"),
                                tr("Statement draft was discarded."), false,
                                QString(), draftId);
    importWorkflow_->clearDraft();
  }
  navigateToImport();
}

void StatementDraftState::addTransactionAfterCurrent() {
  if (!draft_) {
    return;
  }
  draft_->insertTransactionAfterCurrent();
  if (importWorkflow_) {
    importWorkflow_->persistStatementDraft(draft_);
  }
  emit changed();
}

void StatementDraftState::deleteCurrentTransaction() {
  if (!canDeleteTransaction()) {
    return;
  }
  draft_->removeTransaction(draft_->currentIndex());
  if (importWorkflow_) {
    importWorkflow_->persistStatementDraft(draft_);
  }
  emit changed();
}

void StatementDraftState::finalizeDraft() {
  if (!draft_ || !importWorkflow_) {
    return;
  }
  persistPendingEdits();
  const QString draftId = currentDraftId();
  const QString statementId = importWorkflow_->finalizeStatementDraft(draft_);
  if (statementId.isEmpty()) {
    importWorkflow_->addRunNote(tr("Finalize failed"),
                                tr("Draft could not be finalized."), true,
                                QString(), draftId);
    return;
  }
  importWorkflow_->clearPersistedStatementDraft(draftId);
  importWorkflow_->addRunNote(tr("Finalized"),
                              tr("Draft was finalized into a statement."),
                              false, statementId, draftId);
  importWorkflow_->clearDraft();
  navigateToImport();
}

void StatementDraftState::openPreviousTransaction() {
  if (!canOpenPreviousTransaction()) {
    return;
  }
  commitPendingEdits();
  draft_->prev();
}

void StatementDraftState::openNextTransaction() {
  if (!canOpenNextTransaction()) {
    return;
  }
  commitPendingEdits();
  draft_->next();
}

void StatementDraftState::openPreviousDraft() {
  if (!canOpenPreviousDraft()) {
    return;
  }
  persistPendingEdits();
  importWorkflow_->openPrevDraft();
}

void StatementDraftState::openNextDraft() {
  if (!canOpenNextDraft()) {
    return;
  }
  persistPendingEdits();
  importWorkflow_->openNextDraft();
}

void StatementDraftState::bindImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (importWorkflow_) {
    connect(importWorkflow_, &ImportWorkflow::stateChanged, this,
            &StatementDraftState::changed);
  }
}

void StatementDraftState::bindDraft(StatementDraft *value) {
  if (draft_) {
    disconnect(draft_, nullptr, this, nullptr);
  }
  draft_ = value;
  if (draft_) {
    connect(draft_, &StatementDraft::changed, this,
            &StatementDraftState::changed);
  }
}

QString StatementDraftState::currentDraftId() const {
  return draft_ ? draft_->draftId() : QString();
}

void StatementDraftState::navigateToImport() {
  if (navigation_) {
    navigation_->setSection(NavigationState::Section::Import);
  }
}

} // namespace ui

