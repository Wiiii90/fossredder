/**
 * @file ui/include/ui/state/import/TransactionDraftState.h
 * @brief Declares transaction draft UI state used by the import review flow.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

class TransactionDraftState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(TransactionDraftState)

  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(ui::StatementDraft *draft READ draft WRITE setDraft NOTIFY changed)
  Q_PROPERTY(QVariantMap current READ current NOTIFY changed)
  Q_PROPERTY(QVariantMap viewState READ viewState NOTIFY changed)
  Q_PROPERTY(QString nameText READ nameText WRITE setNameText NOTIFY changed)
  Q_PROPERTY(QString bookingDateText READ bookingDateText WRITE
                 setBookingDateText NOTIFY changed)
  Q_PROPERTY(QString valutaText READ valutaText WRITE setValutaText NOTIFY
                 changed)
  Q_PROPERTY(QString amountText READ amountText WRITE setAmountText NOTIFY
                 changed)
  Q_PROPERTY(QVariantList statusOptions READ statusOptions NOTIFY changed)
  Q_PROPERTY(int statusIndex READ statusIndex NOTIFY changed)
  Q_PROPERTY(QString metadataText READ metadataText NOTIFY changed)
  Q_PROPERTY(QString proofSource READ proofSource NOTIFY changed)
  Q_PROPERTY(bool effectiveAllocatable READ effectiveAllocatable NOTIFY changed)
  Q_PROPERTY(QVariantList actorChoiceModel READ actorChoiceModel NOTIFY changed)
  Q_PROPERTY(int selectedActorIndex READ selectedActorIndex NOTIFY changed)
  Q_PROPERTY(QString actorText READ actorText WRITE setActorText NOTIFY changed)
  Q_PROPERTY(bool canAddActor READ canAddActor NOTIFY changed)
  Q_PROPERTY(QVariantList contractChoiceModel READ contractChoiceModel NOTIFY
                 changed)
  Q_PROPERTY(int selectedContractIndex READ selectedContractIndex NOTIFY
                 changed)
  Q_PROPERTY(QString contractNameText READ contractNameText WRITE
                 setContractNameText NOTIFY changed)
  Q_PROPERTY(QString contractTypeText READ contractTypeText WRITE
                 setContractTypeText NOTIFY changed)
  Q_PROPERTY(QString contractNamePlaceholder READ contractNamePlaceholder
                 NOTIFY changed)
  Q_PROPERTY(QVariantList contractAllocatableModes READ contractAllocatableModes
                 CONSTANT)
  Q_PROPERTY(int contractAllocatableModeIndex READ contractAllocatableModeIndex
                 WRITE setContractAllocatableModeIndex NOTIFY changed)
  Q_PROPERTY(QString selectedContractType READ selectedContractType NOTIFY
                 changed)
  Q_PROPERTY(bool canAddContract READ canAddContract NOTIFY changed)
  Q_PROPERTY(QVariantList propertyRows READ propertyRows NOTIFY changed)
  Q_PROPERTY(QString newPropertyName READ newPropertyName WRITE
                 setNewPropertyName NOTIFY changed)
  Q_PROPERTY(bool canAddProperty READ canAddProperty NOTIFY changed)
  Q_PROPERTY(double actorSuggestionConfidence READ actorSuggestionConfidence
                 NOTIFY changed)
  Q_PROPERTY(QString actorSuggestionSummary READ actorSuggestionSummary NOTIFY
                 changed)
  Q_PROPERTY(double propertySuggestionConfidence READ
                 propertySuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString propertySuggestionSummary READ propertySuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double contractSuggestionConfidence READ
                 contractSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString contractSuggestionSummary READ contractSuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double allocatableSuggestionConfidence READ
                 allocatableSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString allocatableSuggestionText READ allocatableSuggestionText
                 NOTIFY changed)

public:
  explicit TransactionDraftState(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }
  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  StatementDraft *draft() const noexcept { return draft_; }

  void setImportWorkflow(ImportWorkflow *value);
  void setWorkspace(WorkspaceFacade *value);
  void setDraft(StatementDraft *value);

  QVariantMap current() const;
  QVariantMap viewState() const { return viewState_; }
  QString nameText() const { return nameText_; }
  void setNameText(const QString &value);
  QString bookingDateText() const { return bookingDateText_; }
  void setBookingDateText(const QString &value);
  QString valutaText() const { return valutaText_; }
  void setValutaText(const QString &value);
  QString amountText() const { return amountText_; }
  void setAmountText(const QString &value);
  QVariantList statusOptions() const;
  int statusIndex() const;
  QString metadataText() const;
  QString proofSource() const;
  bool effectiveAllocatable() const;

  QVariantList actorChoiceModel() const;
  int selectedActorIndex() const;
  QString actorText() const { return actorText_; }
  void setActorText(const QString &value);
  bool canAddActor() const;

  QVariantList contractChoiceModel() const;
  int selectedContractIndex() const;
  QString contractNameText() const { return contractNameText_; }
  void setContractNameText(const QString &value);
  QString contractTypeText() const { return contractTypeText_; }
  void setContractTypeText(const QString &value);
  QString contractNamePlaceholder() const;
  QVariantList contractAllocatableModes() const;
  int contractAllocatableModeIndex() const;
  void setContractAllocatableModeIndex(int index);
  QString selectedContractType() const;
  bool canAddContract() const;

  QVariantList propertyRows() const;
  QString newPropertyName() const { return newPropertyName_; }
  void setNewPropertyName(const QString &value);
  bool canAddProperty() const;

  Q_INVOKABLE void refreshDerivedState();
  Q_INVOKABLE void commitPendingEdits();
  Q_INVOKABLE void commitNameText();
  Q_INVOKABLE void commitBookingDateText();
  Q_INVOKABLE void commitValutaText();
  Q_INVOKABLE void commitAmountText();
  Q_INVOKABLE void setStatusByIndex(int index);
  Q_INVOKABLE int suggestionTone(double confidence) const;
  double actorSuggestionConfidence() const;
  QString actorSuggestionSummary() const;
  double propertySuggestionConfidence() const;
  QString propertySuggestionSummary() const;
  double contractSuggestionConfidence() const;
  QString contractSuggestionSummary() const;
  double allocatableSuggestionConfidence() const;
  QString allocatableSuggestionText() const;
  Q_INVOKABLE void selectActorIndex(int index);
  Q_INVOKABLE void addActorFromText();
  Q_INVOKABLE void selectContractIndex(int index);
  Q_INVOKABLE void addContractFromFields();
  Q_INVOKABLE bool isPropertySelected(const QString &propertyId) const;
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
                                       bool selected);
  Q_INVOKABLE void addPropertyFromInput();
  Q_INVOKABLE void toggleAllocatable();

signals:
  void changed();

private:
  void bindImportWorkflow(ImportWorkflow *value);
  void bindWorkspace(WorkspaceFacade *value);
  void bindDraft(StatementDraft *value);
  void syncTextsForCurrentTransaction();
  void resetTransactionLocalState();
  QVariant value(const QString &key) const;
  QString currentId() const;
  int currentIndex() const;
  QStringList currentPropertyIds() const;
  QString currentActorId() const;
  QString normalizedText(const QString &value) const;
  bool listSetsEqual(const QVariantList &lhs, const QVariantList &rhs) const;
  QVariantList viewRows(const QString &key) const;
  QVariantList workspaceActorRows() const;
  QVariantList localActorRows() const;
  void rememberLocalActorRow(const QVariantMap &row);
  QVariantMap selectedContractRow() const;

  ImportWorkflow *importWorkflow_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
  StatementDraft *draft_ = nullptr;
  QVariantMap viewState_;
  QString lastTransactionId_;
  QString nameText_;
  QString bookingDateText_;
  QString valutaText_;
  QString amountText_;
  QString actorText_;
  QVariantList localActorRows_;
  QVariantList localContractRows_;
  QString contractNameText_;
  QString contractTypeText_;
  QString contractAllocatableMode_ = QStringLiteral("mixed");
  QString newPropertyName_;
};

} // namespace ui
