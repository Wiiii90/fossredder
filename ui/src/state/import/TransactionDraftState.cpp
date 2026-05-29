/**
 * @file ui/src/state/import/TransactionDraftState.cpp
 * @brief Implements transaction draft UI state for import review.
 */

#include "ui/state/import/TransactionDraftState.h"

#include <algorithm>

#include <QRegularExpression>
#include <QSet>

#include "ui/shared/payload/PayloadKeys.h"
#include "ui/viewmodels/import/StatementDraftViewModel.h"
#include "ui/viewmodels/import/TransactionDraftListModel.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr int kStatusNeutral = 0;
constexpr int kStatusUnverified = 1;
constexpr int kStatusVerified = 2;
constexpr int kStatusCompleted = 3;
constexpr int kSuggestionDanger = 0;
constexpr int kSuggestionWarning = 1;
constexpr int kSuggestionSuccess = 2;

QVariantMap mapAt(const QVariantList &rows, int index) {
  return index >= 0 && index < rows.size() ? rows.at(index).toMap()
                                           : QVariantMap{};
}

QString stringValue(const QVariantMap &map, const QString &key) {
  return map.value(key).toString();
}

QVariantList listValue(const QVariantMap &map, const QString &key) {
  const QVariant value = map.value(key);
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    QVariantList out;
    const QStringList strings = value.toStringList();
    out.reserve(strings.size());
    for (const QString &entry : strings) {
      out.push_back(entry);
    }
    return out;
  }
  return value.toList();
}

QStringList stringListValue(const QVariantMap &map, const QString &key) {
  const QVariant value = map.value(key);
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    return value.toStringList();
  }
  QStringList out;
  for (const QVariant &entry : value.toList()) {
    out.push_back(entry.toString());
  }
  return out;
}

QVariantMap choiceRow(const QString &id, const QString &display,
                      const QString &name = {}, const QString &type = {},
                      const QVariantList &actorIds = {},
                      const QVariantList &propertyIds = {},
                      const QString &allocatableMode = QStringLiteral("mixed")) {
  return QVariantMap{{QStringLiteral("id"), id},
                     {QStringLiteral("display"), display},
                     {QStringLiteral("name"), name},
                     {QStringLiteral("type"), type},
                     {QStringLiteral("actorIds"), actorIds},
                     {QStringLiteral("propertyIds"), propertyIds},
                     {QStringLiteral("allocatableMode"), allocatableMode}};
}

} // namespace

TransactionDraftState::TransactionDraftState(QObject *parent)
    : QObject(parent) {}

void TransactionDraftState::setImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_ == value) {
    return;
  }
  bindImportWorkflow(value);
  refreshDerivedState();
  emit changed();
}

void TransactionDraftState::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  emit changed();
}

void TransactionDraftState::setDraft(StatementDraft *value) {
  if (draft_ == value) {
    return;
  }
  bindDraft(value);
  resetTransactionLocalState();
  refreshDerivedState();
  emit changed();
}

QVariantMap TransactionDraftState::current() const {
  return draft_ ? draft_->current() : QVariantMap{};
}

void TransactionDraftState::setNameText(const QString &value) {
  if (nameText_ == value) {
    return;
  }
  nameText_ = value;
  emit changed();
}

void TransactionDraftState::setBookingDateText(const QString &value) {
  if (bookingDateText_ == value) {
    return;
  }
  bookingDateText_ = value;
  emit changed();
}

void TransactionDraftState::setValutaText(const QString &value) {
  if (valutaText_ == value) {
    return;
  }
  valutaText_ = value;
  emit changed();
}

void TransactionDraftState::setAmountText(const QString &value) {
  if (amountText_ == value) {
    return;
  }
  amountText_ = value;
  emit changed();
}

QVariantList TransactionDraftState::statusOptions() const {
  return {QVariantMap{{QStringLiteral("label"), tr("Neutral")},
                      {QStringLiteral("value"), kStatusNeutral}},
          QVariantMap{{QStringLiteral("label"), tr("Unverified")},
                      {QStringLiteral("value"), kStatusUnverified}},
          QVariantMap{{QStringLiteral("label"), tr("Verified")},
                      {QStringLiteral("value"), kStatusVerified}},
          QVariantMap{{QStringLiteral("label"), tr("Completed")},
                      {QStringLiteral("value"), kStatusCompleted}}};
}

int TransactionDraftState::statusIndex() const {
  const int status = value(payload::keys::common::kStatus).toInt();
  const QVariantList options = statusOptions();
  for (int index = 0; index < options.size(); ++index) {
    if (mapAt(options, index).value(QStringLiteral("value")).toInt() ==
        status) {
      return index;
    }
  }
  return 0;
}

QString TransactionDraftState::metadataText() const {
  return value(payload::keys::common::kMetadata).toString();
}

QString TransactionDraftState::proofSource() const {
  const QString data =
      value(payload::keys::transaction::kProofImageData).toString();
  return data.isEmpty() ? QString{} : QStringLiteral("data:image/jpeg;base64,") + data;
}

bool TransactionDraftState::effectiveAllocatable() const {
  const QVariantMap tx = current();
  if (tx.value(payload::keys::draft::kAllocatableSelected).toBool()) {
    return tx.value(payload::keys::transaction::kAllocatable).toBool();
  }
  if (viewState_.contains(QStringLiteral("effectiveAllocatable"))) {
    return viewState_.value(QStringLiteral("effectiveAllocatable")).toBool();
  }
  return tx.value(payload::keys::transaction::kAllocatable).toBool();
}

QVariantList TransactionDraftState::actorChoiceModel() const {
  QVariantList model{choiceRow(QString(), tr("No actor"))};
  QSet<QString> seenIds;
  const auto appendRow = [&](const QVariantMap &row) {
    const QString id = row.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || seenIds.contains(id)) {
      return;
    }
    seenIds.insert(id);
    const QString display = row.value(QStringLiteral("display")).toString();
    model.push_back(choiceRow(id, display.isEmpty()
                                      ? row.value(QStringLiteral("name")).toString()
                                      : display));
  };

  for (const QVariant &row : viewRows(QStringLiteral("actorChoices"))) {
    appendRow(row.toMap());
  }
  for (const QVariant &row : workspaceActorRows()) {
    appendRow(row.toMap());
  }
  for (const QVariant &row : localActorRows()) {
    appendRow(row.toMap());
  }
  return model;
}

int TransactionDraftState::selectedActorIndex() const {
  const QString actorId = currentActorId();
  const QVariantList rows = actorChoiceModel();
  for (int index = 0; index < rows.size(); ++index) {
    if (mapAt(rows, index).value(QStringLiteral("id")).toString() ==
        actorId) {
      return index;
    }
  }
  return 0;
}

void TransactionDraftState::setActorText(const QString &value) {
  if (actorText_ == value) {
    return;
  }
  actorText_ = value;
  emit changed();
}

bool TransactionDraftState::canAddActor() const {
  const QString name = actorText_.trimmed();
  if (name.isEmpty()) {
    return false;
  }
  const QString target = normalizedText(name);
  for (const QVariant &row : actorChoiceModel()) {
    const QVariantMap map = row.toMap();
    if (!map.value(QStringLiteral("id")).toString().isEmpty() &&
        normalizedText(map.value(QStringLiteral("display")).toString()) ==
            target) {
      return false;
    }
  }
  return true;
}

QVariantList TransactionDraftState::contractChoiceModel() const {
  QVariantList model{choiceRow(QString(), tr("No contract"))};
  QSet<QString> seenIds;
  const auto appendRow = [&](const QVariantMap &row) {
    const QString id = row.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || seenIds.contains(id)) {
      return;
    }
    seenIds.insert(id);
    const QString name = row.value(QStringLiteral("name")).toString();
    const QString type = row.value(QStringLiteral("type")).toString();
    QString display = row.value(QStringLiteral("display")).toString();
    if (!name.isEmpty()) {
      display = name;
    } else if (display.isEmpty()) {
      display = type;
    }
    model.push_back(choiceRow(
        id, display, name, type, listValue(row, QStringLiteral("actorIds")),
        listValue(row, QStringLiteral("propertyIds")),
        row.value(QStringLiteral("allocatableMode"),
                  QStringLiteral("mixed"))
            .toString()));
  };

  for (const QVariant &row : localContractRows_) {
    appendRow(row.toMap());
  }
  for (const QVariant &row : viewRows(QStringLiteral("contractChoices"))) {
    appendRow(row.toMap());
  }
  return model;
}

int TransactionDraftState::selectedContractIndex() const {
  const QString contractId = value(payload::keys::transaction::kContractId).toString();
  const QVariantList rows = contractChoiceModel();
  for (int index = 0; index < rows.size(); ++index) {
    if (mapAt(rows, index).value(QStringLiteral("id")).toString() ==
        contractId) {
      return index;
    }
  }
  return 0;
}

void TransactionDraftState::setContractNameText(const QString &value) {
  if (contractNameText_ == value) {
    return;
  }
  contractNameText_ = value;
  emit changed();
}

void TransactionDraftState::setContractTypeText(const QString &value) {
  if (contractTypeText_ == value) {
    return;
  }
  contractTypeText_ = value;
  emit changed();
}

QString TransactionDraftState::contractNamePlaceholder() const {
  int maxIndex = 0;
  const QRegularExpression pattern(QStringLiteral("^Contract\\s+(\\d+)$"),
                                   QRegularExpression::CaseInsensitiveOption);
  for (const QVariant &value : contractChoiceModel()) {
    const QString name = value.toMap().value(QStringLiteral("name")).toString().trimmed();
    const QRegularExpressionMatch match = pattern.match(name);
    if (match.hasMatch()) {
      const int index = match.captured(1).toInt();
      maxIndex = std::max(maxIndex, index);
    }
  }
  return tr("Contract %1").arg(maxIndex + 1);
}

QVariantList TransactionDraftState::contractAllocatableModes() const {
  return {QVariantMap{{QStringLiteral("label"), tr("Mixed")},
                      {QStringLiteral("value"), QStringLiteral("mixed")}},
          QVariantMap{{QStringLiteral("label"), tr("All allocatable")},
                      {QStringLiteral("value"), QStringLiteral("allocatable")}},
          QVariantMap{{QStringLiteral("label"), tr("Never allocatable")},
                      {QStringLiteral("value"),
                       QStringLiteral("non-allocatable")}}};
}

int TransactionDraftState::contractAllocatableModeIndex() const {
  const QVariantList rows = contractAllocatableModes();
  for (int index = 0; index < rows.size(); ++index) {
    if (mapAt(rows, index).value(QStringLiteral("value")).toString() ==
        contractAllocatableMode_) {
      return index;
    }
  }
  return 0;
}

void TransactionDraftState::setContractAllocatableModeIndex(int index) {
  const QVariantMap row = mapAt(contractAllocatableModes(), index);
  const QString mode =
      row.value(QStringLiteral("value"), QStringLiteral("mixed")).toString();
  if (contractAllocatableMode_ == mode) {
    return;
  }
  contractAllocatableMode_ = mode;
  emit changed();
}

QString TransactionDraftState::selectedContractType() const {
  return selectedContractRow().value(QStringLiteral("type")).toString();
}

bool TransactionDraftState::canAddContract() const {
  if (normalizedText(contractTypeText_).isEmpty()) {
    return false;
  }
  const QString type = normalizedText(contractTypeText_);
  const QString name = normalizedText(contractNameText_);
  const QString actorId = currentActorId();
  const QVariantList targetActorIds = actorId.isEmpty()
                                          ? QVariantList{}
                                          : QVariantList{actorId};
  QVariantList targetPropertyIds;
  for (const QString &propertyId : currentPropertyIds()) {
    targetPropertyIds.push_back(propertyId);
  }
  for (const QVariant &value : contractChoiceModel()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString().isEmpty()) {
      continue;
    }
    if (normalizedText(row.value(QStringLiteral("type")).toString()) != type) {
      continue;
    }
    if (!listSetsEqual(listValue(row, QStringLiteral("actorIds")),
                       targetActorIds)) {
      continue;
    }
    if (!listSetsEqual(listValue(row, QStringLiteral("propertyIds")),
                       targetPropertyIds)) {
      continue;
    }
    if (!name.isEmpty() &&
        normalizedText(row.value(QStringLiteral("name")).toString()) != name) {
      continue;
    }
    return false;
  }
  return true;
}

QVariantList TransactionDraftState::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList{};
}

void TransactionDraftState::setNewPropertyName(const QString &value) {
  if (newPropertyName_ == value) {
    return;
  }
  newPropertyName_ = value;
  emit changed();
}

bool TransactionDraftState::canAddProperty() const {
  const QString name = normalizedText(newPropertyName_);
  if (name.isEmpty()) {
    return false;
  }
  for (const QVariant &value : propertyRows()) {
    const QVariantMap row = value.toMap();
    if (normalizedText(row.value(QStringLiteral("display"),
                                 row.value(QStringLiteral("name")))
                           .toString()) == name) {
      return false;
    }
  }
  return true;
}

void TransactionDraftState::refreshDerivedState() {
  viewState_ = importWorkflow_ && draft_
                   ? importWorkflow_->currentTransactionViewState(draft_)
                   : QVariantMap{};
  emit changed();
}

void TransactionDraftState::commitPendingEdits() {
  commitNameText();
  commitBookingDateText();
  commitValutaText();
  commitAmountText();
}

void TransactionDraftState::commitNameText() {
  if (draft_ && draft_->transactions() && nameText_ != value(payload::keys::common::kName).toString()) {
    draft_->transactions()->setName(currentIndex(), nameText_);
  }
}

void TransactionDraftState::commitBookingDateText() {
  if (draft_ && draft_->transactions() &&
      bookingDateText_ != value(payload::keys::transaction::kBookingDate).toString()) {
    draft_->transactions()->setBookingDate(currentIndex(), bookingDateText_);
  }
}

void TransactionDraftState::commitValutaText() {
  if (draft_ && draft_->transactions() &&
      valutaText_ != value(payload::keys::transaction::kValuta).toString()) {
    draft_->transactions()->setValuta(currentIndex(), valutaText_);
  }
}

void TransactionDraftState::commitAmountText() {
  if (importWorkflow_ && draft_) {
    importWorkflow_->updateCurrentAmount(draft_, amountText_);
  }
}

void TransactionDraftState::setStatusByIndex(int index) {
  if (!draft_ || !draft_->transactions()) {
    return;
  }
  const QVariantMap row = mapAt(statusOptions(), index);
  draft_->transactions()->setStatus(
      currentIndex(), row.value(QStringLiteral("value"), kStatusNeutral).toInt());
  draft_->refresh();
}

int TransactionDraftState::suggestionTone(double confidence) const {
  if (confidence >= 0.9) {
    return kSuggestionSuccess;
  }
  if (confidence >= 0.4) {
    return kSuggestionWarning;
  }
  return kSuggestionDanger;
}

double TransactionDraftState::actorSuggestionConfidence() const {
  return viewState_.value(QStringLiteral("actorSuggestionConfidence")).toDouble();
}

QString TransactionDraftState::actorSuggestionSummary() const {
  const QString value = viewState_.value(QStringLiteral("actorSuggestionSummary")).toString();
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

double TransactionDraftState::propertySuggestionConfidence() const {
  return viewState_.value(QStringLiteral("propertySuggestionConfidence")).toDouble();
}

QString TransactionDraftState::propertySuggestionSummary() const {
  const QString value = viewState_.value(QStringLiteral("propertySuggestionSummary")).toString();
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

double TransactionDraftState::contractSuggestionConfidence() const {
  return viewState_.value(QStringLiteral("contractSuggestionConfidence")).toDouble();
}

QString TransactionDraftState::contractSuggestionSummary() const {
  const QString value = viewState_.value(QStringLiteral("contractSuggestionSummary")).toString();
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

double TransactionDraftState::allocatableSuggestionConfidence() const {
  return viewState_.value(QStringLiteral("allocatableSuggestionConfidence")).toDouble();
}

QString TransactionDraftState::allocatableSuggestionText() const {
  const QString value =
      viewState_.value(QStringLiteral("allocatableSuggestionSummary")).toString();
  return value.isEmpty() ? tr("0% Confidence - No suggestion") : value;
}

void TransactionDraftState::selectActorIndex(int index) {
  if (!importWorkflow_ || !draft_) {
    return;
  }
  const QVariantMap row = mapAt(actorChoiceModel(), index);
  const QString id = row.value(QStringLiteral("id")).toString();
  if (!id.isEmpty()) {
    rememberLocalActorRow(row);
    importWorkflow_->selectCurrentActorChoice(draft_, row);
    draft_->transactions()->setContractId(currentIndex(), QString());
    draft_->transactions()->setContractSelected(currentIndex(), false);
    draft_->refresh();
    return;
  }
  draft_->transactions()->setActorId(currentIndex(), QString());
  draft_->transactions()->setActorText(currentIndex(), QString());
  draft_->transactions()->setActorSelected(currentIndex(), false);
  draft_->transactions()->setContractId(currentIndex(), QString());
  draft_->transactions()->setContractSelected(currentIndex(), false);
  draft_->refresh();
}

void TransactionDraftState::addActorFromText() {
  if (!canAddActor() || !importWorkflow_ || !draft_) {
    return;
  }
  const QVariantMap row =
      importWorkflow_->createActorChoiceForCurrentDraft(draft_, actorText_);
  if (!row.value(QStringLiteral("id")).toString().isEmpty()) {
    rememberLocalActorRow(row);
    importWorkflow_->selectCurrentActorChoice(draft_, row);
    draft_->refresh();
  }
}

void TransactionDraftState::selectContractIndex(int index) {
  if (!importWorkflow_ || !draft_) {
    return;
  }
  const QVariantMap row = mapAt(contractChoiceModel(), index);
  if (!row.value(QStringLiteral("id")).toString().isEmpty()) {
    importWorkflow_->selectCurrentContractChoice(draft_, row);
  } else {
    draft_->transactions()->setContractId(currentIndex(), QString());
    draft_->transactions()->setContractSelected(currentIndex(), false);
    draft_->refresh();
  }
}

void TransactionDraftState::addContractFromFields() {
  if (!canAddContract() || !importWorkflow_ || !draft_) {
    return;
  }
  const QVariantMap row = importWorkflow_->createOrSelectContractChoiceForCurrentDraft(
      draft_, contractNameText_, contractTypeText_, contractAllocatableMode_);
  if (!row.value(QStringLiteral("id")).toString().isEmpty()) {
    localContractRows_.prepend(row);
    importWorkflow_->selectCurrentContractChoice(draft_, row);
    contractNameText_ = row.value(QStringLiteral("name"),
                                  row.value(QStringLiteral("display")))
                            .toString();
    emit changed();
  }
}

bool TransactionDraftState::isPropertySelected(const QString &propertyId) const {
  return currentPropertyIds().contains(propertyId);
}

void TransactionDraftState::setPropertySelected(const QString &propertyId,
                                                bool selected) {
  if (importWorkflow_ && draft_) {
    importWorkflow_->setCurrentPropertySelected(draft_, propertyId, selected);
  }
}

void TransactionDraftState::addPropertyFromInput() {
  if (!canAddProperty() || !importWorkflow_ || !draft_) {
    return;
  }
  importWorkflow_->createPropertyChoiceForCurrentDraft(draft_, newPropertyName_);
  newPropertyName_.clear();
  emit changed();
}

void TransactionDraftState::toggleAllocatable() {
  if (!draft_ || !draft_->transactions()) {
    return;
  }
  draft_->transactions()->setAllocatable(currentIndex(), !effectiveAllocatable());
  draft_->transactions()->setAllocatableSelected(currentIndex(), true);
  draft_->refresh();
}

void TransactionDraftState::bindImportWorkflow(ImportWorkflow *value) {
  if (importWorkflow_) {
    disconnect(importWorkflow_, nullptr, this, nullptr);
  }
  importWorkflow_ = value;
  if (importWorkflow_) {
    connect(importWorkflow_, &ImportWorkflow::stateChanged, this,
            &TransactionDraftState::refreshDerivedState);
  }
}

void TransactionDraftState::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (workspace_) {
    connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
            &TransactionDraftState::refreshDerivedState);
  }
}

void TransactionDraftState::bindDraft(StatementDraft *value) {
  if (draft_) {
    disconnect(draft_, nullptr, this, nullptr);
  }
  draft_ = value;
  if (draft_) {
    connect(draft_, &StatementDraft::changed, this, [this]() {
      syncTextsForCurrentTransaction();
      refreshDerivedState();
    });
  }
}

void TransactionDraftState::syncTextsForCurrentTransaction() {
  const QString id = currentId();
  if (id == lastTransactionId_) {
    emit changed();
    return;
  }
  lastTransactionId_ = id;
  resetTransactionLocalState();
  emit changed();
}

void TransactionDraftState::resetTransactionLocalState() {
  const QVariantMap tx = current();
  lastTransactionId_ = stringValue(tx, payload::keys::common::kId);
  nameText_ = stringValue(tx, payload::keys::common::kName);
  bookingDateText_ = stringValue(tx, payload::keys::transaction::kBookingDate);
  valutaText_ = stringValue(tx, payload::keys::transaction::kValuta);
  amountText_ = tx.contains(payload::keys::common::kAmount)
                    ? tx.value(payload::keys::common::kAmount).toString()
                    : QString();
  actorText_.clear();
  localActorRows_.clear();
  localContractRows_.clear();
  contractNameText_.clear();
  contractTypeText_.clear();
  contractAllocatableMode_ = QStringLiteral("mixed");
  newPropertyName_.clear();
}

QVariant TransactionDraftState::value(const QString &key) const {
  return current().value(key);
}

QString TransactionDraftState::currentId() const {
  return value(payload::keys::common::kId).toString();
}

int TransactionDraftState::currentIndex() const {
  return draft_ ? draft_->currentIndex() : -1;
}

QStringList TransactionDraftState::currentPropertyIds() const {
  return stringListValue(current(), payload::keys::transaction::kPropertyIds);
}

QString TransactionDraftState::currentActorId() const {
  return value(payload::keys::transaction::kActorId).toString();
}

QString TransactionDraftState::normalizedText(const QString &value) const {
  QString normalized = value.trimmed().toLower();
  normalized.replace(QRegularExpression(QStringLiteral("\\s+")),
                     QStringLiteral(" "));
  return normalized;
}

bool TransactionDraftState::listSetsEqual(const QVariantList &lhs,
                                          const QVariantList &rhs) const {
  QStringList left;
  QStringList right;
  for (const QVariant &value : lhs) {
    left.push_back(value.toString());
  }
  for (const QVariant &value : rhs) {
    right.push_back(value.toString());
  }
  left.sort();
  right.sort();
  return left == right;
}

QVariantList TransactionDraftState::viewRows(const QString &key) const {
  return viewState_.value(key).toList();
}

QVariantList TransactionDraftState::workspaceActorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList{};
}

QVariantList TransactionDraftState::localActorRows() const {
  return localActorRows_;
}

void TransactionDraftState::rememberLocalActorRow(const QVariantMap &row) {
  const QString id = row.value(QStringLiteral("id")).toString();
  if (id.isEmpty()) {
    return;
  }
  for (QVariant &item : localActorRows_) {
    QVariantMap current = item.toMap();
    if (current.value(QStringLiteral("id")).toString() == id) {
      item = row;
      return;
    }
  }
  localActorRows_.push_back(row);
}

QVariantMap TransactionDraftState::selectedContractRow() const {
  return mapAt(contractChoiceModel(), selectedContractIndex());
}

} // namespace ui
