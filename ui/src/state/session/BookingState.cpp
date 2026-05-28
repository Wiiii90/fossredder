/**
 * @file ui/src/state/session/BookingState.cpp
 * @brief Implements the selection-aware booking UI state wrapper.
 */

#include "ui/state/session/BookingState.h"

#include "ui/state/session/WorkspaceSessionSelection.h"
#include "ui/state/session/WorkspaceSessionState.h"
#include "ui/workspace/WorkspaceFacade.h"

#include <cmath>

namespace ui {

namespace {

constexpr int kStatusNeutral = 0;
constexpr int kStatusUnverified = 1;
constexpr int kStatusVerified = 2;
constexpr int kStatusCompleted = 3;

QStringList stringsFromVariantList(const QVariantList &values) {
  QStringList out;
  out.reserve(values.size());
  for (const QVariant &value : values) {
    out.push_back(value.toString());
  }
  return out;
}

QVariantList variantStringList(const QVariant &value) {
  if (value.metaType() == QMetaType::fromType<QStringList>()) {
    QVariantList out;
    const QStringList strings = value.toStringList();
    out.reserve(strings.size());
    for (const QString &string : strings) {
      out.push_back(string);
    }
    return out;
  }
  return value.toList();
}

} // namespace

BookingState::BookingState(WorkspaceFacade *workspace, QObject *parent)
    : QObject(parent), workspace_(workspace) {
  createTransactions_ = QVariantList{emptyTransaction()};
  currentTransactionDraft_ = emptyTransaction();
  bindSignals();
  if (isCreateMode()) {
    resetCreateState();
  } else {
    syncEditState();
  }
}

bool BookingState::isCreateMode() const {
  return !workspace_ || workspace_->selectedStatementId().isEmpty();
}

QString BookingState::statementName() const {
  return isCreateMode() ? createStatementName_ : editStatementName_;
}

void BookingState::setStatementName(const QString &value) {
  if (isCreateMode()) {
    if (createStatementName_ == value) {
      return;
    }
    createStatementName_ = value;
  } else {
    if (editStatementName_ == value) {
      return;
    }
    editStatementName_ = value;
  }
  emit changed();
}

QVariantMap BookingState::transactionData() const {
  return isCreateMode() ? currentTransactionDraft_ : editTransactionData_;
}

void BookingState::setTransactionData(const QVariantMap &value) {
  if (isCreateMode()) {
    if (!workspace_ || !workspace_->session()) {
      return;
    }
    const QVariantMap draft = value.isEmpty() ? emptyTransaction() : value;
    const QVariantMap state = workspace_->session()->setCurrentRawDraft(
        createTransactions_, createTransactionIndex_, draft, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
  } else {
    if (editTransactionData_ == value) {
      return;
    }
    editTransactionData_ = value.isEmpty() ? emptyTransaction() : value;
  }
  emit changed();
}

QVariant BookingState::value(const QString &key) const {
  return transactionData().value(key);
}

void BookingState::setTransactionValue(const QString &key,
                                       const QVariant &value) {
  QVariantMap next = transactionData();
  next.insert(key, value);
  setTransactionData(next);
}

void BookingState::applyTransactionDraftChange(const QVariantMap &changes) {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  setTransactionData(workspace_->session()->transactionDraft(
      transactionData(), contractRows(), changes));
}

QString BookingState::transactionName() const {
  return value(QStringLiteral("name")).toString();
}

void BookingState::setTransactionName(const QString &value) {
  setTransactionValue(QStringLiteral("name"), value);
}

QString BookingState::transactionBookingDate() const {
  return value(QStringLiteral("bookingDate")).toString();
}

void BookingState::setTransactionBookingDate(const QString &value) {
  setTransactionValue(QStringLiteral("bookingDate"), value);
}

QString BookingState::transactionValuta() const {
  return value(QStringLiteral("valuta")).toString();
}

void BookingState::setTransactionValuta(const QString &value) {
  setTransactionValue(QStringLiteral("valuta"), value);
}

QString BookingState::transactionAmountText() const {
  const QVariant amount = value(QStringLiteral("amount"));
  return amount.isValid() ? amount.toString() : QString();
}

void BookingState::setTransactionAmountText(const QString &value) {
  setTransactionValue(QStringLiteral("amount"), value);
}

int BookingState::transactionStatusIndex() const {
  if (!workspace_ || !workspace_->session()) {
    return 0;
  }
  const QVariantMap tx = normalizeTransaction(transactionData());
  const int index = workspace_->session()->indexOfKeyValue(
      transactionStatusOptions(), QStringLiteral("value"),
      tx.value(QStringLiteral("status"), kStatusNeutral));
  return index >= 0 ? index : 0;
}

void BookingState::setTransactionStatusIndex(int index) {
  const QVariantList options = transactionStatusOptions();
  const QVariantMap option = options.value(index).toMap();
  setTransactionValue(QStringLiteral("status"),
                      option.value(QStringLiteral("value"), kStatusNeutral));
}

bool BookingState::transactionAllocatable() const {
  return value(QStringLiteral("allocatable")).toBool();
}

void BookingState::setTransactionAllocatable(bool value) {
  setTransactionValue(QStringLiteral("allocatable"), value);
}

QVariantList BookingState::actorRows() const {
  return workspace_ ? workspace_->actorRows() : QVariantList();
}

QVariantList BookingState::contractRows() const {
  return workspace_ ? workspace_->contractRows() : QVariantList();
}

QVariantList BookingState::propertyRows() const {
  return workspace_ ? workspace_->propertyRows() : QVariantList();
}

QVariantList BookingState::selectedPropertyIds() const {
  return normalizeTransaction(transactionData())
      .value(QStringLiteral("propertyIds"))
      .toList();
}

QVariantList BookingState::transactionStatusOptions() const {
  return {
      QVariantMap{{QStringLiteral("label"), tr("Neutral")},
                  {QStringLiteral("value"), kStatusNeutral}},
      QVariantMap{{QStringLiteral("label"), tr("Unverified")},
                  {QStringLiteral("value"), kStatusUnverified}},
      QVariantMap{{QStringLiteral("label"), tr("Verified")},
                  {QStringLiteral("value"), kStatusVerified}},
      QVariantMap{{QStringLiteral("label"), tr("Completed")},
                  {QStringLiteral("value"), kStatusCompleted}},
  };
}

QVariantList BookingState::statementRows() const {
  if (!workspace_) {
    return QVariantList();
  }

  QVariantList rows = workspace_->statementRows();
  for (QVariant &rowValue : rows) {
    QVariantMap row = rowValue.toMap();
    const QString statementId = row.value(QStringLiteral("id")).toString();
    row.insert(QStringLiteral("transactions"),
               workspace_->statementTransactionRows(statementId));
    rowValue = row;
  }
  return rows;
}

QString BookingState::selectedStatementId() const {
  return workspace_ ? workspace_->selectedStatementId() : QString();
}

QString BookingState::selectedTransactionId() const {
  return workspace_ ? workspace_->selectedTransactionId() : QString();
}

QVariantList BookingState::actorDisplayRows() const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->displayRowsWithEmpty(
                   actorRows(), tr("No actor"), QStringLiteral("display"))
             : QVariantList();
}

QVariantList BookingState::contractDisplayRows() const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->displayRowsWithEmpty(
                   contractRows(), tr("No contract"), QStringLiteral("display"))
             : QVariantList();
}

int BookingState::selectedIndexFor(const QVariantList &rows,
                                   const QString &id) const {
  if (!workspace_ || !workspace_->session()) {
    return 0;
  }
  const int index = workspace_->session()->indexOfId(rows, id);
  return index >= 0 ? index : 0;
}

int BookingState::selectedActorIndex() const {
  return selectedIndexFor(actorDisplayRows(),
                          value(QStringLiteral("actorId")).toString());
}

int BookingState::selectedContractIndex() const {
  return selectedIndexFor(contractDisplayRows(),
                          value(QStringLiteral("contractId")).toString());
}

QString BookingState::transactionInfoText() const {
  if (isCreateMode()) {
    return createTransactions_.isEmpty()
               ? tr("No transactions")
               : tr("Transaction %1 / %2")
                     .arg(createTransactionIndex_ + 1)
                     .arg(createTransactions_.size());
  }

  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  if (rows.isEmpty()) {
    return tr("No transactions");
  }
  const int index = state.value(QStringLiteral("index")).toInt();
  return tr("Transaction %1 / %2").arg(index + 1).arg(rows.size());
}

bool BookingState::hasStatements() const { return !statementRows().isEmpty(); }

bool BookingState::hasMultipleTransactions() const {
  return isCreateMode()
             ? createTransactions_.size() > 1
             : editTransactionState().value(QStringLiteral("rows")).toList().size() > 1;
}

bool BookingState::canCreate() const {
  if (createStatementName_.trimmed().isEmpty()) {
    return false;
  }
  for (const QVariant &draftValue : createTransactions_) {
    if (!transactionDraftCanSubmit(draftValue.toMap())) {
      return false;
    }
  }
  return true;
}

bool BookingState::canUpdate() const {
  if (isCreateMode() || !workspace_ || !workspace_->session()) {
    return false;
  }
  return workspace_->session()->bookingEditStateChanged(
      savedEditStatementName_, savedEditTransactionJson_, editStatementName_,
      editTransactionData_);
}

bool BookingState::canAddTransaction() const {
  return isCreateMode() || (workspace_ && !workspace_->selectedStatementId().isEmpty());
}

bool BookingState::canDeleteTransaction() const {
  if (isCreateMode()) {
    return createTransactions_.size() > 1;
  }
  return hasMultipleTransactions() &&
         !editTransactionData_.value(QStringLiteral("id")).toString().isEmpty();
}

QVariantMap BookingState::emptyTransaction() const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->emptyTransactionDraft()
             : QVariantMap();
}

QVariantMap BookingState::normalizeTransaction(const QVariantMap &tx) const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->normalizeTransactionDraft(tx)
             : QVariantMap();
}

QVariantList BookingState::statementTransactionRows(const QString &statementId) const {
  return workspace_ ? workspace_->statementTransactionRows(statementId)
                    : QVariantList();
}

QVariantMap BookingState::editTransactionState() const {
  if (!workspace_ || !workspace_->session() || workspace_->selectedStatementId().isEmpty()) {
    return {{QStringLiteral("rows"), QVariantList()},
            {QStringLiteral("orderIds"), QVariantList()},
            {QStringLiteral("index"), -1},
            {QStringLiteral("id"), QString()}};
  }
  const QVariantList rows = statementTransactionRows(workspace_->selectedStatementId());
  const QVariantList preferred =
      editTransactionOrderIds_.isEmpty() ? workspace_->session()->rowIds(rows)
                                         : editTransactionOrderIds_;
  return workspace_->session()->orderedSelectionState(
      rows, preferred, editTransactionIndex_, workspace_->selectedTransactionId(),
      QStringLiteral("id"));
}

QVariantMap BookingState::transactionById(const QString &txId) const {
  if (!workspace_ || txId.isEmpty()) {
    return emptyTransaction();
  }
  QVariantMap transaction = workspace_->transaction(txId);
  transaction.insert(QStringLiteral("propertyIds"),
                     variantStringList(transaction.value(QStringLiteral("propertyIds"))));
  return normalizeTransaction(transaction);
}

QString BookingState::transactionSnapshot(const QVariantMap &data) const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->transactionDraftSnapshot(data)
             : QString();
}

bool BookingState::transactionDraftCanSubmit(const QVariantMap &draft) const {
  if (!workspace_ || !workspace_->session()) {
    return false;
  }

  const QVariantMap tx = normalizeTransaction(draft);
  if (!workspace_->session()->transactionDraftHasContent(tx)) {
    return true;
  }

  const QString bookingDate =
      tx.value(QStringLiteral("bookingDate")).toString().trimmed();
  if (bookingDate.isEmpty()) {
    return false;
  }

  const double amount = workspace_->session()->amountForTransactionCommit(
      draft.value(QStringLiteral("amount")), QString(),
      tx.value(QStringLiteral("amount")).toDouble());
  return std::isfinite(amount);
}

QVariantList BookingState::submittableTransactionDrafts() const {
  QVariantList out;
  if (!workspace_ || !workspace_->session()) {
    return out;
  }

  for (const QVariant &draftValue : createTransactions_) {
    const QVariantMap draft = draftValue.toMap();
    const QVariantMap normalized = normalizeTransaction(draft);
    if (!workspace_->session()->transactionDraftHasContent(normalized)) {
      continue;
    }
    if (!transactionDraftCanSubmit(draft)) {
      continue;
    }
    out.push_back(draft);
  }
  return out;
}

void BookingState::captureEditState() {
  savedEditStatementName_ = editStatementName_;
  savedEditTransactionJson_ = transactionSnapshot(editTransactionData_);
}

void BookingState::rememberSelectedTransaction() {
  if (!workspace_ || workspace_->selectedStatementId().isEmpty() ||
      workspace_->selectedTransactionId().isEmpty()) {
    return;
  }
  lastTransactionIdByStatementId_.insert(workspace_->selectedStatementId(),
                                         workspace_->selectedTransactionId());
}

QString BookingState::transactionIdForStatement(const QString &statementId) const {
  return workspace_ && workspace_->session()
             ? workspace_->session()->rememberedOrFirstRowId(
                   statementTransactionRows(statementId),
                   lastTransactionIdByStatementId_, statementId,
                   QStringLiteral("id"))
             : QString();
}

bool BookingState::ensureSelectedTransactionForStatement() {
  if (isCreateMode() || !workspace_ || workspace_->selectedStatementId().isEmpty()) {
    return true;
  }
  const QString statementId = workspace_->selectedStatementId();
  const QString currentTxId = workspace_->selectedTransactionId();
  if (workspace_->session()->rowHasId(statementTransactionRows(statementId),
                                      currentTxId, QStringLiteral("id"))) {
    return true;
  }
  const QString nextTxId = transactionIdForStatement(statementId);
  if (currentTxId == nextTxId) {
    return true;
  }
  workspace_->selection()->setSelectedTransactionId(nextTxId);
  return false;
}

void BookingState::currentCreateTransaction() {
  if (!workspace_ || !workspace_->session()) {
    createTransactions_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionDraft_ = emptyTransaction();
    return;
  }
  const QVariantMap state = workspace_->session()->currentDraftState(
      createTransactions_, createTransactionIndex_, emptyTransaction());
  createTransactions_ = state.value(QStringLiteral("drafts")).toList();
  createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
  currentTransactionDraft_ = state.value(QStringLiteral("draft")).toMap();
}

void BookingState::syncEditState() {
  if (isCreateMode()) {
    return;
  }
  if (!ensureSelectedTransactionForStatement()) {
    return;
  }
  editStatementName_ =
      workspace_->selectedStatement() ? workspace_->selectedStatement()->name() : QString();
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    editTransactionIndex_ = -1;
    editTransactionData_ = emptyTransaction();
    emit changed();
    return;
  }
  editTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
  const QString txId = state.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selection()->setSelectedTransactionId(txId);
  rememberSelectedTransaction();
  captureEditState();
  emit changed();
}

void BookingState::resetCreateState() {
  createStatementName_.clear();
  if (workspace_ && workspace_->session()) {
    const QVariantMap state = workspace_->session()->createDraftListState(
        QVariantList(), 0, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentTransactionDraft_ = state.value(QStringLiteral("draft")).toMap();
  } else {
    createTransactions_ = QVariantList{emptyTransaction()};
    createTransactionIndex_ = 0;
    currentTransactionDraft_ = emptyTransaction();
  }
  emit changed();
}

void BookingState::previousStatement() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = statementRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString currentId = isCreateMode() ? QString() : workspace_->selectedStatementId();
  const int currentIndex = workspace_->session()->indexOfId(rows, currentId);
  if (currentIndex == 0) {
    rememberSelectedTransaction();
    workspace_->selection()->setSelectedStatementId(QString());
    workspace_->selection()->setSelectedTransactionId(QString());
    editTransactionOrderIds_.clear();
    editTransactionIndex_ = -1;
    emit changed();
    return;
  }
  const int nextIndex = currentIndex < 0 ? rows.size() - 1 : currentIndex - 1;
  const QString nextId = rows.value(nextIndex).toMap().value(QStringLiteral("id")).toString();
  rememberSelectedTransaction();
  workspace_->selection()->setSelectedStatementId(nextId);
  workspace_->selection()->setSelectedTransactionId(transactionIdForStatement(nextId));
  editTransactionOrderIds_.clear();
  editTransactionIndex_ = 0;
  emit changed();
}

void BookingState::nextStatement() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  const QVariantList rows = statementRows();
  if (rows.isEmpty()) {
    return;
  }
  const QString currentId = isCreateMode() ? QString() : workspace_->selectedStatementId();
  const int currentIndex = workspace_->session()->indexOfId(rows, currentId);
  if (currentIndex == rows.size() - 1) {
    rememberSelectedTransaction();
    workspace_->selection()->setSelectedStatementId(QString());
    workspace_->selection()->setSelectedTransactionId(QString());
    editTransactionOrderIds_.clear();
    editTransactionIndex_ = -1;
    emit changed();
    return;
  }
  const int nextIndex = currentIndex < 0 ? 0 : currentIndex + 1;
  const QString nextId = rows.value(nextIndex).toMap().value(QStringLiteral("id")).toString();
  rememberSelectedTransaction();
  workspace_->selection()->setSelectedStatementId(nextId);
  workspace_->selection()->setSelectedTransactionId(transactionIdForStatement(nextId));
  editTransactionOrderIds_.clear();
  editTransactionIndex_ = 0;
  emit changed();
}

void BookingState::previousTransaction() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  if (isCreateMode()) {
    if (createTransactions_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        workspace_->session()->wrappedIndex(createTransactionIndex_ - 1,
                                            createTransactions_.size());
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    return;
  }
  const QVariantMap next = workspace_->session()->navigateSelectionState(
      rows, state.value(QStringLiteral("index")).toInt(),
      state.value(QStringLiteral("id")).toString(), -1, 0, QStringLiteral("id"));
  editTransactionIndex_ = next.value(QStringLiteral("index")).toInt();
  const QString txId = next.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selection()->setSelectedTransactionId(txId);
  rememberSelectedTransaction();
  emit changed();
}

void BookingState::nextTransaction() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  if (isCreateMode()) {
    if (createTransactions_.isEmpty()) {
      return;
    }
    createTransactionIndex_ =
        workspace_->session()->wrappedIndex(createTransactionIndex_ + 1,
                                            createTransactions_.size());
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QVariantMap state = editTransactionState();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  if (rows.isEmpty()) {
    return;
  }
  const QVariantMap next = workspace_->session()->navigateSelectionState(
      rows, state.value(QStringLiteral("index")).toInt(),
      state.value(QStringLiteral("id")).toString(), 1, 0, QStringLiteral("id"));
  editTransactionIndex_ = next.value(QStringLiteral("index")).toInt();
  const QString txId = next.value(QStringLiteral("id")).toString();
  editTransactionData_ = transactionById(txId);
  workspace_->selection()->setSelectedTransactionId(txId);
  rememberSelectedTransaction();
  emit changed();
}

void BookingState::addTransactionAfterCurrent() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  if (isCreateMode()) {
    const QVariantMap state = workspace_->session()->insertDraftAfterCurrent(
        createTransactions_, createTransactionIndex_, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
    emit changed();
    return;
  }

  if (workspace_->selectedStatementId().isEmpty()) {
    return;
  }
  const QVariantMap state = editTransactionState();
  editTransactionOrderIds_ = state.value(QStringLiteral("orderIds")).toList();
  const QVariantList rows = state.value(QStringLiteral("rows")).toList();
  int insertAfterIndex = state.value(QStringLiteral("index"), editTransactionIndex_).toInt();
  if (insertAfterIndex < 0 || insertAfterIndex >= rows.size()) {
    insertAfterIndex = rows.size() - 1;
  }
  const QVariantMap current = normalizeTransaction(editTransactionData_);
  const QString newId = workspace_->insertTransactionAfter(
      current.value(QStringLiteral("id")).toString(), QString(),
      current.value(QStringLiteral("bookingDate")).toString(),
      current.value(QStringLiteral("valuta")).toString(), 0.0,
      workspace_->selectedStatementId(), 0, QString(), QString(), false,
      QStringList());
  if (newId.isEmpty()) {
    return;
  }
  const QVariantList updatedRows = statementTransactionRows(workspace_->selectedStatementId());
  const QVariantList updatedIds = workspace_->session()->rowIds(updatedRows);
  editTransactionOrderIds_ = workspace_->session()->orderWithInsertedId(
      editTransactionOrderIds_, updatedIds, newId, insertAfterIndex);
  workspace_->selection()->setSelectedTransactionId(newId);
  syncEditState();
}

void BookingState::deleteCurrentTransaction() {
  if (!workspace_ || !workspace_->session()) {
    return;
  }
  if (isCreateMode()) {
    const QVariantMap state = workspace_->session()->removeDraftAt(
        createTransactions_, createTransactionIndex_, emptyTransaction());
    createTransactions_ = state.value(QStringLiteral("drafts")).toList();
    createTransactionIndex_ = state.value(QStringLiteral("index")).toInt();
    currentCreateTransaction();
    emit changed();
    return;
  }
  const QString deletedId = editTransactionData_.value(QStringLiteral("id")).toString();
  if (deletedId.isEmpty() || !hasMultipleTransactions()) {
    return;
  }
  const QVariantMap selectionState = editTransactionState();
  editTransactionOrderIds_ = selectionState.value(QStringLiteral("orderIds")).toList();
  workspace_->deleteTransaction(deletedId);
  const QVariantMap reselectionState = workspace_->session()->deleteReselectionState(
      statementTransactionRows(workspace_->selectedStatementId()),
      editTransactionOrderIds_, editTransactionIndex_, deletedId, QStringLiteral("id"));
  editTransactionOrderIds_ = reselectionState.value(QStringLiteral("orderIds")).toList();
  editTransactionIndex_ = reselectionState.value(QStringLiteral("index")).toInt();
  const QString txId = reselectionState.value(QStringLiteral("id")).toString();
  workspace_->selection()->setSelectedTransactionId(txId);
  rememberSelectedTransaction();
  editTransactionData_ = transactionById(txId);
  emit changed();
}

bool BookingState::isPropertySelected(const QString &propertyId) const {
  for (const QVariant &value : selectedPropertyIds()) {
    if (value.toString() == propertyId) {
      return true;
    }
  }
  return false;
}

void BookingState::setPropertySelected(const QString &propertyId,
                                       bool selected) {
  if (!workspace_ || !workspace_->session() || propertyId.trimmed().isEmpty()) {
    return;
  }
  const QVariantList propertyIds =
      normalizeTransaction(transactionData()).value(QStringLiteral("propertyIds")).toList();
  const QVariantList next =
      selected ? workspace_->session()->addUniqueTrimmed(propertyIds, propertyId)
               : workspace_->session()->removeString(propertyIds, propertyId);
  applyTransactionDraftChange({{QStringLiteral("propertyIds"), next}});
}

void BookingState::selectActorIndex(int index) {
  const QVariantList rows = actorDisplayRows();
  const QString id = rows.value(index).toMap().value(QStringLiteral("id")).toString();
  applyTransactionDraftChange({{QStringLiteral("actorId"), id}});
}

void BookingState::selectContractIndex(int index) {
  const QVariantList rows = contractDisplayRows();
  const QString id = rows.value(index).toMap().value(QStringLiteral("id")).toString();
  applyTransactionDraftChange({{QStringLiteral("contractId"), id}});
}

void BookingState::selectStatement(const QString &statementId) {
  if (!workspace_ || !workspace_->selection()) {
    return;
  }
  workspace_->selection()->setSelectedStatementId(statementId);
  workspace_->selection()->setSelectedTransactionId(QString());
}

void BookingState::selectTransaction(const QString &statementId,
                                     const QString &transactionId) {
  if (!workspace_ || !workspace_->selection()) {
    return;
  }
  workspace_->selection()->setSelectedStatementId(statementId);
  workspace_->selection()->setSelectedTransactionId(transactionId);
}

QString BookingState::submit() {
  if (!workspace_ || !canCreate()) {
    return QString();
  }
  const QString statementName = createStatementName_;
  const QVariantList transactionDrafts = submittableTransactionDrafts();
  const QString statementId = workspace_->addStatement(statementName);
  if (statementId.isEmpty()) {
    return QString();
  }
  for (const QVariant &draftValue : transactionDrafts) {
    const QVariantMap rawTx = draftValue.toMap();
    const QVariantMap tx = normalizeTransaction(rawTx);
    workspace_->addTransaction(
        tx.value(QStringLiteral("name")).toString(),
        tx.value(QStringLiteral("bookingDate")).toString(),
        tx.value(QStringLiteral("valuta")).toString(),
        workspace_->session()->amountForTransactionCommit(
            rawTx.value(QStringLiteral("amount")), QString(),
            tx.value(QStringLiteral("amount")).toDouble()),
        statementId, tx.value(QStringLiteral("status")).toInt(),
        tx.value(QStringLiteral("actorId")).toString(),
        tx.value(QStringLiteral("contractId")).toString(),
        tx.value(QStringLiteral("allocatable")).toBool(),
        stringsFromVariantList(tx.value(QStringLiteral("propertyIds")).toList()));
  }
  resetCreateState();
  workspace_->selection()->setSelectedStatementId(statementId);
  workspace_->selection()->setSelectedTransactionId(transactionIdForStatement(statementId));
  return statementId;
}

void BookingState::updateCurrent() {
  if (isCreateMode() || !workspace_ || workspace_->selectedStatementId().isEmpty()) {
    return;
  }
  const QString selectedStatementId = workspace_->selectedStatementId();
  const bool statementChanged = savedEditStatementName_ != editStatementName_;
  const QString statementName = editStatementName_;
  const QVariantMap transactionData = editTransactionData_;
  const QString txId = transactionData.value(QStringLiteral("id")).toString();
  const bool transactionChanged =
      savedEditTransactionJson_ != transactionSnapshot(transactionData);
  const QVariantMap normalizedTx = normalizeTransaction(transactionData);
  if (transactionChanged && !txId.isEmpty()) {
    workspace_->updateTransaction(
        txId, transactionData.value(QStringLiteral("name")).toString(),
        transactionData.value(QStringLiteral("bookingDate")).toString(),
        transactionData.value(QStringLiteral("valuta")).toString(),
        workspace_->session()->amountForTransactionCommit(
            transactionData.value(QStringLiteral("amount")), txId,
            normalizedTx.value(QStringLiteral("amount")).toDouble()),
        selectedStatementId, normalizedTx.value(QStringLiteral("status")).toInt(),
        transactionData.value(QStringLiteral("actorId")).toString(),
        transactionData.value(QStringLiteral("contractId")).toString(),
        transactionData.value(QStringLiteral("allocatable")).toBool(),
        stringsFromVariantList(transactionData.value(QStringLiteral("propertyIds")).toList()));
  }
  if (statementChanged) {
    workspace_->updateStatement(selectedStatementId, statementName);
  }
  syncEditState();
}

void BookingState::deleteCurrentStatement() {
  if (isCreateMode() || !workspace_) {
    return;
  }
  const QString removedId = workspace_->selectedStatementId();
  workspace_->deleteStatement(removedId);
  const QString nextId = workspace_->session()->deleteNextSelectionId(
      statementRows(), removedId, 0, QStringLiteral("id"));
  workspace_->selection()->setSelectedStatementId(nextId);
  workspace_->selection()->setSelectedTransactionId(nextId.isEmpty() ? QString() : transactionIdForStatement(nextId));
  emit changed();
}

void BookingState::bindSignals() {
  if (!workspace_ || !workspace_->selection()) {
    return;
  }
  QObject::connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this, [this]() {
    if (isCreateMode()) {
      resetCreateState();
    } else {
      syncEditState();
    }
  });
  QObject::connect(workspace_->selection(), &SessionSelection::selectedStatementIdChanged,
                   this, [this]() {
                     if (isCreateMode()) {
                       resetCreateState();
                     } else {
                       editTransactionOrderIds_.clear();
                       editTransactionIndex_ = 0;
                       syncEditState();
                     }
                   });
  QObject::connect(workspace_->selection(), &SessionSelection::selectedTransactionIdChanged,
                   this, [this]() {
                     if (isCreateMode()) {
                       resetCreateState();
                     } else {
                       rememberSelectedTransaction();
                       syncEditState();
                     }
                   });
}

} // namespace ui
