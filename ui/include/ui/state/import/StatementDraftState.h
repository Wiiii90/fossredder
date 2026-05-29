/**
 * @file ui/include/ui/state/import/StatementDraftState.h
 * @brief Declares statement draft review state used by the import view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

#include "ui/state/import/TransactionDraftState.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"

namespace ui {

class StatementDraftState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(StatementDraftState)

  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY changed)
  Q_PROPERTY(ui::StatementDraft *draft READ draft WRITE setDraft NOTIFY changed)
  Q_PROPERTY(ui::TransactionDraftState *transactionState READ transactionState
                 WRITE setTransactionState NOTIFY changed)
  Q_PROPERTY(bool hasDraft READ hasDraft NOTIFY changed)
  Q_PROPERTY(QString statementName READ statementName WRITE setStatementName
                 NOTIFY changed)
  Q_PROPERTY(QString transactionInfoText READ transactionInfoText NOTIFY changed)
  Q_PROPERTY(bool canAddTransaction READ canAddTransaction NOTIFY changed)
  Q_PROPERTY(bool canDeleteTransaction READ canDeleteTransaction NOTIFY changed)
  Q_PROPERTY(bool canOpenPreviousTransaction READ canOpenPreviousTransaction
                 NOTIFY changed)
  Q_PROPERTY(bool canOpenNextTransaction READ canOpenNextTransaction NOTIFY
                 changed)
  Q_PROPERTY(bool canOpenPreviousDraft READ canOpenPreviousDraft NOTIFY changed)
  Q_PROPERTY(bool canOpenNextDraft READ canOpenNextDraft NOTIFY changed)

public:
  explicit StatementDraftState(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }
  NavigationState *navigation() const noexcept { return navigation_; }
  StatementDraft *draft() const noexcept { return draft_; }
  TransactionDraftState *transactionState() const noexcept {
    return transactionState_;
  }

  void setImportWorkflow(ImportWorkflow *value);
  void setNavigation(NavigationState *value);
  void setDraft(StatementDraft *value);
  void setTransactionState(TransactionDraftState *value);

  bool hasDraft() const noexcept;
  QString statementName() const;
  void setStatementName(const QString &value);
  QString transactionInfoText() const;
  bool canAddTransaction() const noexcept;
  bool canDeleteTransaction() const noexcept;
  bool canOpenPreviousTransaction() const noexcept;
  bool canOpenNextTransaction() const noexcept;
  bool canOpenPreviousDraft() const noexcept;
  bool canOpenNextDraft() const noexcept;

  Q_INVOKABLE void commitPendingEdits();
  Q_INVOKABLE void persistPendingEdits();
  Q_INVOKABLE void returnToImport();
  Q_INVOKABLE void discardDraft();
  Q_INVOKABLE void addTransactionAfterCurrent();
  Q_INVOKABLE void deleteCurrentTransaction();
  Q_INVOKABLE void finalizeDraft();
  Q_INVOKABLE void openPreviousTransaction();
  Q_INVOKABLE void openNextTransaction();
  Q_INVOKABLE void openPreviousDraft();
  Q_INVOKABLE void openNextDraft();

signals:
  void changed();

private:
  void bindImportWorkflow(ImportWorkflow *value);
  void bindDraft(StatementDraft *value);
  QString currentDraftId() const;
  void navigateToImport();

  ImportWorkflow *importWorkflow_ = nullptr;
  NavigationState *navigation_ = nullptr;
  StatementDraft *draft_ = nullptr;
  TransactionDraftState *transactionState_ = nullptr;
};

} // namespace ui
