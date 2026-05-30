/**
 * @file ui/src/state/session/AnnualState.cpp
 * @brief Implements the UI state adapter for the Annual view.
 */

#include "ui/state/session/AnnualState.h"

#include <algorithm>
#include <cmath>

#include <QDate>
#include <QMetaType>
#include <QSet>

#include "ui/workflows/annual/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr int kYearMin = 1900;
constexpr int kYearMax = 2500;
constexpr auto kDeduplicated = "deduplicated";
constexpr auto kSimilar = "similar";
constexpr auto kDivergent = "divergent";
constexpr auto kWorkspaceOnly = "workspaceOnly";
constexpr auto kMissingLive = "missingLive";
constexpr auto kTab = "tab";
constexpr auto kPlot = "plot";
constexpr auto kXlsx = "xlsx";
constexpr auto kCsv = "csv";
constexpr auto kPng = "png";
constexpr auto kJpg = "jpg";

QString qstr(const char *value) { return QString::fromLatin1(value); }

double numberFromVariant(const QVariant &value) {
  bool ok = false;
  const double out = value.toDouble(&ok);
  return ok && std::isfinite(out) ? out : 0.0;
}

QString amountText(const QVariant &value) {
  return QString::number(numberFromVariant(value), 'f', 2);
}

QString nonEmptyString(const QVariantMap &map, const QString &key,
                       const QString &fallback = {}) {
  const QString value = map.value(key).toString();
  return value.isEmpty() ? fallback : value;
}

} // namespace

AnnualState::AnnualState(QObject *parent) : QObject(parent) {
  sectionExpanded_.insert(qstr(kDeduplicated), true);
  sectionExpanded_.insert(qstr(kSimilar), true);
  sectionExpanded_.insert(qstr(kDivergent), true);
  sectionExpanded_.insert(qstr(kWorkspaceOnly), true);
  sectionExpanded_.insert(qstr(kMissingLive), true);
  verificationIssues_ = emptyIssues();
  statusMetrics_ = emptyStatusMetrics();
  year_ = defaultYear();
}

void AnnualState::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  refreshFromSelection();
  emitChanged();
}

void AnnualState::setAnnualWorkflow(AnnualWorkflow *value) {
  if (annualWorkflow_ == value) {
    return;
  }
  annualWorkflow_ = value;
  lastPreviewRevision_ = -1;
  refreshFromSelection();
  emitChanged();
}

bool AnnualState::isEdit() const { return !selectedAnnualId().isEmpty(); }

QVariantList AnnualState::annualRows() const {
  return workspace_ ? workspace_->annualRows() : QVariantList();
}

QString AnnualState::selectedAnnualId() const {
  return workspace_ ? workspace_->selectedAnnualId() : QString();
}

void AnnualState::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emitChanged();
}

void AnnualState::setYear(int value) {
  const int next = std::clamp(value, kYearMin, kYearMax);
  if (year_ == next) {
    return;
  }
  year_ = next;
  rebuildAnnualResultState();
  emitChanged();
}

QVariantList AnnualState::analysisIds() const {
  QVariantList out;
  out.reserve(analysisIds_.size());
  for (const QString &id : analysisIds_) {
    out.push_back(id);
  }
  return out;
}

QVariantList AnnualState::transactionSections() const {
  return {
      transactionSection(qstr(kDeduplicated), tr("Included entries (exact matches)"),
                         annualTransactionGroups_.value(qstr(kDeduplicated)).toList()),
      transactionSection(qstr(kSimilar), tr("Included entries (possible variants)"),
                         annualTransactionGroups_.value(qstr(kSimilar)).toList()),
      transactionSection(qstr(kDivergent), tr("Included entries (unique)"),
                         annualTransactionGroups_.value(qstr(kDivergent)).toList()),
      transactionSection(qstr(kWorkspaceOnly),
                         tr("Missing live transactions from selected year"),
                         annualTransactionGroups_.value(qstr(kWorkspaceOnly)).toList()),
      transactionSection(qstr(kMissingLive), tr("Included deleted transactions"),
                         missingLiveRows()),
  };
}

QVariantList AnnualState::verificationRows() const {
  return {
      QVariantMap{{QStringLiteral("label"), tr("Included transactions")},
                  {QStringLiteral("value"), annualTransactions_.size()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{{QStringLiteral("label"), tr("Duplicate entries")},
                  {QStringLiteral("value"),
                   verificationIssues_.value(QStringLiteral("duplicateCount")).toInt()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{{QStringLiteral("label"),
                   tr("Missing live transactions from selected year")},
                  {QStringLiteral("value"),
                   verificationIssues_.value(QStringLiteral("missingFromYear")).toInt()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{{QStringLiteral("label"), tr("Included deleted transactions")},
                  {QStringLiteral("value"),
                   verificationIssues_.value(QStringLiteral("missingLive")).toInt()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{{QStringLiteral("label"),
                   tr("Transactions from a different year than selected")},
                  {QStringLiteral("value"),
                   verificationIssues_.value(QStringLiteral("mixedInAnnual")).toInt()},
                  {QStringLiteral("tone"),
                   verificationIssues_.value(QStringLiteral("mixedInAnnual")).toInt() == 0
                       ? QStringLiteral("success")
                       : QStringLiteral("danger")}},
      QVariantMap{{QStringLiteral("label"), tr("Assigned analyses")},
                  {QStringLiteral("value"), analysisIds_.size()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
  };
}

QString AnnualState::statusSummaryText() const {
  return tr("Neutral: %1, Unverified: %2, Verified: %3, Completed: %4")
      .arg(statusMetrics_.value(QStringLiteral("neutral")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("unverified")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("verified")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("completed")).toInt());
}

bool AnnualState::hasRows() const { return !annualRows().isEmpty(); }

bool AnnualState::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  QStringList current = analysisIds_;
  QStringList saved = savedAnalysisIds_;
  current.sort();
  saved.sort();
  return savedName_ != name_ || savedYear_ != year_ || saved != current ||
         analysisMetadataDirty_;
}

void AnnualState::refreshFromSelection() {
  loadSelectedAnnual();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualState::resetCreateState() {
  if (isEdit() && workspace_) {
    workspace_->setSelectedAnnualId({});
  }
  loadCreateState();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualState::submitCreate() {
  if (!workspace_ || !canSubmit()) {
    return;
  }
  const QString id = workspace_->saveAnnual({}, name_, year_, analysisIds_);
  if (id.isEmpty()) {
    return;
  }
  workspace_->setSelectedAnnualId(id);
  captureSavedState();
  emitChanged();
}

void AnnualState::submitUpdate() {
  if (!workspace_ || selectedAnnualId().isEmpty() || !canSubmit()) {
    return;
  }
  workspace_->saveAnnual(selectedAnnualId(), name_, year_, analysisIds_);
  captureSavedState();
  emitChanged();
}

void AnnualState::deleteCurrent() {
  if (!workspace_ || selectedAnnualId().isEmpty()) {
    return;
  }
  const QString removedId = selectedAnnualId();
  const QVariantList rows = annualRows();
  workspace_->deleteAnnual(removedId);

  QString nextId;
  for (const QVariant &value : rows) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty() && id != removedId) {
      nextId = id;
      break;
    }
  }
  workspace_->setSelectedAnnualId(nextId);
  if (nextId.isEmpty()) {
    loadCreateState();
  }
  emitChanged();
}

void AnnualState::navigate(int delta) {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = annualRows();
  if (rows.isEmpty()) {
    return;
  }
  const int lastIndex = static_cast<int>(rows.size()) - 1;
  const int currentIndex = annualIndexById(selectedAnnualId());
  if (currentIndex < 0) {
    const int target = delta < 0 ? lastIndex : 0;
    workspace_->setSelectedAnnualId(
        rows.at(target).toMap().value(QStringLiteral("id")).toString());
    return;
  }
  if ((delta > 0 && currentIndex == lastIndex) || (delta < 0 && currentIndex == 0)) {
    workspace_->setSelectedAnnualId({});
    return;
  }
  const int next = std::clamp(currentIndex + delta, 0, lastIndex);
  workspace_->setSelectedAnnualId(
      rows.at(next).toMap().value(QStringLiteral("id")).toString());
}

void AnnualState::selectAnnual(const QString &id) {
  if (workspace_) {
    workspace_->setSelectedAnnualId(id.trimmed());
  }
}

void AnnualState::toggleWorkspace() {
  workspaceIndex_ = workspaceIndex_ == 0 ? 1 : 0;
  emitChanged();
}

void AnnualState::stepYear(int delta) { setYear(year_ + delta); }

void AnnualState::addAvailableAnalysisAtIndex(int index) {
  if (index < 0 || index >= availableAnalysisRows_.size()) {
    return;
  }
  const QString id =
      availableAnalysisRows_.at(index).toMap().value(QStringLiteral("id")).toString();
  if (id.isEmpty() || analysisIds_.contains(id)) {
    return;
  }
  QStringList next = analysisIds_;
  next.push_back(id);
  setAnalysisIds(next);
}

void AnnualState::removeAnalysis(const QString &id) {
  const QString trimmed = id.trimmed();
  if (trimmed.isEmpty() || !analysisIds_.contains(trimmed)) {
    return;
  }
  QStringList next = analysisIds_;
  next.removeAll(trimmed);
  setAnalysisIds(next);
}

void AnnualState::setAnalysisExportFormat(const QString &id,
                                          const QString &exportFormat) {
  if (!workspace_) {
    return;
  }
  const QVariantMap row = analysisRowById(id.trimmed());
  if (row.isEmpty()) {
    return;
  }
  const QString type = nonEmptyString(row, QStringLiteral("type"), qstr(kTab));
  const QString nextFormat = normalizedExportFormat(exportFormat, type);
  workspace_->updateAnalysis(
      id.trimmed(), row.value(QStringLiteral("name")).toString(), type,
      row.value(QStringLiteral("config"), QStringLiteral("{}")).toString(),
      row.value(QStringLiteral("filter"),
                row.value(QStringLiteral("filterSpec")).toString())
          .toString(),
      nextFormat,
      row.value(QStringLiteral("includeCalcAdjustments"), true).toBool(),
      row.value(QStringLiteral("exportState"), QStringLiteral("{}")).toString(),
      row.value(QStringLiteral("snapshotTransactions"),
                row.value(QStringLiteral("snapshotTransactionsJson"),
                          QStringLiteral("[]")))
          .toString());
  analysisMetadataDirty_ = true;
  refreshChoices();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualState::toggleTransactionSection(const QString &key) {
  const QString trimmed = key.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  sectionExpanded_.insert(trimmed, !isTransactionSectionExpanded(trimmed));
  emitChanged();
}

bool AnnualState::isTransactionSectionExpanded(const QString &key) const {
  return sectionExpanded_.value(key.trimmed(), true).toBool();
}

QStringList AnnualState::normalizedAnalysisIds(const QVariant &values) const {
  QStringList out;
  QSet<QString> seen;
  const QVariantList list = values.toList();
  if (!list.isEmpty()) {
    for (const QVariant &value : list) {
      const QString id = analysisIdFromVariant(value);
      if (id.isEmpty() || seen.contains(id)) {
        continue;
      }
      seen.insert(id);
      out.push_back(id);
    }
    return out;
  }
  const QString single = analysisIdFromVariant(values);
  if (!single.isEmpty()) {
    out.push_back(single);
  }
  return out;
}

QString AnnualState::analysisIdFromVariant(const QVariant &value) const {
  if (!value.isValid()) {
    return {};
  }
  if (value.typeId() == QMetaType::QString) {
    return value.toString().trimmed();
  }
  const QVariantMap map = value.toMap();
  if (!map.isEmpty()) {
    const QString id = map.value(QStringLiteral("id")).toString().trimmed();
    return id.isEmpty()
               ? map.value(QStringLiteral("objectId")).toString().trimmed()
               : id;
  }
  return value.toString().trimmed();
}

QVariantMap AnnualState::annualRowById(const QString &id) const {
  for (const QVariant &value : annualRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() == id) {
      return row;
    }
  }
  return {};
}

QVariantMap AnnualState::analysisRowById(const QString &id) const {
  for (const QVariant &value : normalizedAnalysisRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() == id) {
      return row;
    }
  }
  return {};
}

int AnnualState::annualIndexById(const QString &id) const {
  if (id.isEmpty()) {
    return -1;
  }
  const QVariantList rows = annualRows();
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(QStringLiteral("id")).toString() == id) {
      return i;
    }
  }
  return -1;
}

QVariantMap AnnualState::normalizeAnalysisRow(const QVariant &value) const {
  const QVariantMap source = value.toMap();
  const QString id = analysisIdFromVariant(value);
  if (id.isEmpty()) {
    return {};
  }
  const QString name = source.value(QStringLiteral("name")).toString();
  const QString display = source.value(QStringLiteral("display")).toString();
  const QString type = nonEmptyString(source, QStringLiteral("type"), qstr(kTab));
  QVariantMap out;
  out.insert(QStringLiteral("id"), id);
  out.insert(QStringLiteral("name"), name);
  out.insert(QStringLiteral("display"),
             !display.isEmpty() ? display : (!name.isEmpty() ? name : id));
  out.insert(QStringLiteral("type"), type);
  out.insert(QStringLiteral("config"),
             source.value(QStringLiteral("config"), QStringLiteral("{}")));
  out.insert(QStringLiteral("filter"),
             source.value(QStringLiteral("filter"),
                          source.value(QStringLiteral("filterSpec"), QString())));
  out.insert(QStringLiteral("exportFormat"),
             source.value(QStringLiteral("exportFormat"), QString()));
  out.insert(QStringLiteral("includeCalcAdjustments"),
             source.value(QStringLiteral("includeCalcAdjustments"), true));
  out.insert(QStringLiteral("exportState"),
             source.value(QStringLiteral("exportState"), QStringLiteral("{}")));
  out.insert(QStringLiteral("snapshotTransactions"),
             source.value(QStringLiteral("snapshotTransactions"),
                          source.value(QStringLiteral("snapshotTransactionsJson"),
                                       QStringLiteral("[]"))));
  return augmentAnalysisRow(out);
}

QVariantList AnnualState::normalizedAnalysisRows() const {
  QVariantList rows;
  if (!workspace_) {
    return rows;
  }
  const QVariantList source = workspace_->analysisRows();
  rows.reserve(source.size());
  for (const QVariant &value : source) {
    const QVariantMap row = normalizeAnalysisRow(value);
    if (!row.isEmpty()) {
      rows.push_back(row);
    }
  }
  return rows;
}

QVariantList AnnualState::buildAssignedAnalysisRows() const {
  const QVariantList rows = normalizedAnalysisRows();
  QVariantList out;
  out.reserve(analysisIds_.size());
  for (const QString &id : analysisIds_) {
    for (const QVariant &value : rows) {
      const QVariantMap row = value.toMap();
      if (row.value(QStringLiteral("id")).toString() == id) {
        out.push_back(row);
        break;
      }
    }
  }
  return out;
}

QVariantList AnnualState::buildAvailableAnalysisRows() const {
  const QVariantList rows = normalizedAnalysisRows();
  const QSet<QString> selected(analysisIds_.begin(), analysisIds_.end());
  QVariantList out;
  for (const QVariant &value : rows) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty() && !selected.contains(id)) {
      out.push_back(value);
    }
  }
  return out;
}

QVariantMap AnnualState::augmentAnalysisRow(const QVariantMap &row) const {
  QVariantMap out = row;
  const QString type = row.value(QStringLiteral("type")).toString().toLower();
  const QVariantList options = exportOptionsForType(type);
  out.insert(QStringLiteral("typeLabel"), type == qstr(kPlot) ? tr("Plot")
                                                               : tr("Table"));
  out.insert(QStringLiteral("exportFormatOptions"), options);
  out.insert(QStringLiteral("exportFormatIndex"),
             exportFormatIndex(options,
                               row.value(QStringLiteral("exportFormat")).toString()));
  return out;
}

QVariantList AnnualState::exportOptionsForType(const QString &type) const {
  if (type.trimmed().toLower() == qstr(kPlot)) {
    return {QStringLiteral("PNG"), QStringLiteral("JPG")};
  }
  return {QStringLiteral("XLSX"), QStringLiteral("CSV")};
}

int AnnualState::exportFormatIndex(const QVariantList &options,
                                   const QString &exportFormat) const {
  const QString normalized = exportFormat.trimmed().toUpper();
  for (int i = 0; i < options.size(); ++i) {
    if (options.at(i).toString().toUpper() == normalized) {
      return i;
    }
  }
  return 0;
}

QString AnnualState::normalizedExportFormat(const QString &value,
                                            const QString &type) const {
  const QString normalized = value.trimmed().toLower();
  const QString normalizedType = type.trimmed().toLower();
  const QStringList allowed = normalizedType == qstr(kPlot)
                                  ? QStringList{qstr(kPng), qstr(kJpg)}
                                  : QStringList{qstr(kXlsx), qstr(kCsv)};
  return allowed.contains(normalized) ? normalized : allowed.first();
}

int AnnualState::defaultYear() const { return QDate::currentDate().year() - 1; }

void AnnualState::loadCreateState() {
  name_.clear();
  year_ = defaultYear();
  analysisIds_.clear();
  workspaceIndex_ = 0;
  analysisMetadataDirty_ = false;
  refreshChoices();
  captureSavedState();
}

void AnnualState::loadSelectedAnnual() {
  if (!workspace_ || selectedAnnualId().isEmpty()) {
    loadCreateState();
    return;
  }
  const QVariantMap payload = workspace_->annual(selectedAnnualId());
  if (payload.isEmpty()) {
    loadCreateState();
    return;
  }
  name_ = payload.value(QStringLiteral("name")).toString();
  year_ = payload.value(QStringLiteral("year"), defaultYear()).toInt();
  analysisIds_ = normalizedAnalysisIds(payload.value(QStringLiteral("analysisIds")));
  analysisMetadataDirty_ = false;
  refreshChoices();
  captureSavedState();
}

void AnnualState::refreshChoices() {
  assignedAnalysisRows_ = buildAssignedAnalysisRows();
  availableAnalysisRows_ = buildAvailableAnalysisRows();
}

void AnnualState::rebuildAnnualResultState() {
  const QString currentAnnualId = selectedAnnualId();
  const int currentRevision = workspaceRevision();
  if (lastPreviewAnnualId_ == currentAnnualId && lastPreviewYear_ == year_ &&
      lastPreviewAnalysisIds_ == analysisIds_ &&
      lastPreviewRevision_ == currentRevision) {
    return;
  }
  lastPreviewAnnualId_ = currentAnnualId;
  lastPreviewYear_ = year_;
  lastPreviewAnalysisIds_ = analysisIds_;
  lastPreviewRevision_ = currentRevision;
  if (!annualWorkflow_) {
    applyAnnualResult({});
    return;
  }
  applyAnnualResult(
      annualWorkflow_->computeAnnualPreview(currentAnnualId, analysisIds_, year_));
}

void AnnualState::applyAnnualResult(const QVariantMap &result) {
  const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();
  verificationIssues_ = QVariantMap{
      {QStringLiteral("missingFromYear"),
       stats.value(QStringLiteral("missingFromYear"), 0)},
      {QStringLiteral("mixedInAnnual"), stats.value(QStringLiteral("mixedYear"), 0)},
      {QStringLiteral("duplicateCount"),
       stats.value(QStringLiteral("duplicateCount"), 0)},
      {QStringLiteral("missingLive"), stats.value(QStringLiteral("missingLive"), 0)}};
  statusMetrics_ = QVariantMap{
      {QStringLiteral("neutral"), stats.value(QStringLiteral("neutral"), 0)},
      {QStringLiteral("unverified"), stats.value(QStringLiteral("unverified"), 0)},
      {QStringLiteral("verified"), stats.value(QStringLiteral("verified"), 0)},
      {QStringLiteral("completed"), stats.value(QStringLiteral("completed"), 0)}};

  annualTransactions_ =
      rowsFromResultBucket(result.value(QStringLiteral("transactions")).toList());
  annualTransactionGroups_ = QVariantMap{
      {qstr(kDeduplicated),
       rowsFromResultBucket(result.value(qstr(kDeduplicated)).toList())},
      {qstr(kSimilar), rowsFromResultBucket(result.value(qstr(kSimilar)).toList())},
      {qstr(kDivergent),
       rowsFromResultBucket(result.value(qstr(kDivergent)).toList())},
      {qstr(kWorkspaceOnly),
       rowsFromResultBucket(result.value(qstr(kWorkspaceOnly)).toList())}};

  if (groupedCount() == 0 && !annualTransactions_.isEmpty()) {
    QVariantList deduplicated;
    QVariantList similar;
    QVariantList divergent;
    QVariantList workspaceOnly;
    for (const QVariant &value : annualTransactions_) {
      const QVariantMap row = value.toMap();
      const QString key = row.value(QStringLiteral("key")).toString();
      if (key.startsWith(QStringLiteral("live|"))) {
        workspaceOnly.push_back(row);
      } else if (key.startsWith(QStringLiteral("sim|")) ||
                 row.value(QStringLiteral("isCalcVariant")).toBool()) {
        similar.push_back(row);
      } else if (key.startsWith(QStringLiteral("div|"))) {
        divergent.push_back(row);
      } else {
        deduplicated.push_back(row);
      }
    }
    annualTransactionGroups_.insert(qstr(kDeduplicated), deduplicated);
    annualTransactionGroups_.insert(qstr(kSimilar), similar);
    annualTransactionGroups_.insert(qstr(kDivergent), divergent);
    annualTransactionGroups_.insert(qstr(kWorkspaceOnly), workspaceOnly);
  }
}

void AnnualState::captureSavedState() {
  savedName_ = name_;
  savedYear_ = year_;
  savedAnalysisIds_ = analysisIds_;
  analysisMetadataDirty_ = false;
}

void AnnualState::setAnalysisIds(const QStringList &ids) {
  QStringList next;
  QSet<QString> seen;
  for (const QString &id : ids) {
    const QString trimmed = id.trimmed();
    if (trimmed.isEmpty() || seen.contains(trimmed)) {
      continue;
    }
    seen.insert(trimmed);
    next.push_back(trimmed);
  }
  if (analysisIds_ == next) {
    return;
  }
  analysisIds_ = next;
  refreshChoices();
  rebuildAnnualResultState();
  emitChanged();
}

QVariantMap AnnualState::emptyIssues() const {
  return {{QStringLiteral("missingFromYear"), 0},
          {QStringLiteral("mixedInAnnual"), 0},
          {QStringLiteral("duplicateCount"), 0},
          {QStringLiteral("missingLive"), 0}};
}

QVariantMap AnnualState::emptyStatusMetrics() const {
  return {{QStringLiteral("neutral"), 0},
          {QStringLiteral("unverified"), 0},
          {QStringLiteral("verified"), 0},
          {QStringLiteral("completed"), 0}};
}

QVariantList AnnualState::rowsFromResultBucket(const QVariantList &rows) const {
  QVariantList out;
  out.reserve(rows.size());
  for (const QVariant &value : rows) {
    out.push_back(transactionRow(value.toMap()));
  }
  return out;
}

QVariantList AnnualState::missingLiveRows() const {
  QVariantList out;
  QSet<QString> seen;
  const QStringList buckets{qstr(kDeduplicated), qstr(kSimilar), qstr(kDivergent)};
  for (const QString &bucket : buckets) {
    for (const QVariant &value : annualTransactionGroups_.value(bucket).toList()) {
      const QVariantMap row = value.toMap();
      if (!row.value(QStringLiteral("isMissingLive")).toBool()) {
        continue;
      }
      const QString key = nonEmptyString(row, QStringLiteral("key"),
                                         row.value(QStringLiteral("id")).toString());
      if (key.isEmpty() || seen.contains(key)) {
        continue;
      }
      seen.insert(key);
      out.push_back(row);
    }
  }
  return out;
}

QVariantMap AnnualState::transactionRow(const QVariantMap &source) const {
  QVariantMap row = source;
  const QVariantList names = source.value(QStringLiteral("sourceAnalysisNames")).toList();
  QStringList nameStrings;
  nameStrings.reserve(names.size());
  for (const QVariant &value : names) {
    const QString text = value.toString();
    if (!text.isEmpty()) {
      nameStrings.push_back(text);
    }
  }
  const int status = source.value(QStringLiteral("status")).toInt();
  row.insert(QStringLiteral("contractType"),
             source.value(QStringLiteral("contractType")).toString());
  row.insert(QStringLiteral("amountText"),
             amountText(source.value(QStringLiteral("amount"))));
  row.insert(QStringLiteral("sourceNamesText"), nameStrings.join(QStringLiteral(", ")));
  row.insert(QStringLiteral("allocatableText"),
             source.value(QStringLiteral("allocatable")).toBool()
                 ? tr("Allocatable")
                 : tr("Non-allocatable"));
  row.insert(QStringLiteral("contractTypeLabel"),
             nonEmptyString(source, QStringLiteral("contractType"),
                            tr("No type assigned")));
  row.insert(QStringLiteral("statusText"),
             nonEmptyString(source, QStringLiteral("statusText"), tr("Neutral")));
  row.insert(QStringLiteral("statusTone"),
             status == 3 ? QStringLiteral("success")
                         : status == 2 ? QStringLiteral("info")
                                       : status == 1 ? QStringLiteral("warning")
                                                     : QStringLiteral("primary"));
  return row;
}

QVariantMap AnnualState::transactionSection(const QString &key,
                                            const QString &title,
                                            const QVariantList &rows) const {
  return {{QStringLiteral("key"), key},
          {QStringLiteral("title"), title},
          {QStringLiteral("rows"), rows},
          {QStringLiteral("expanded"), isTransactionSectionExpanded(key)},
          {QStringLiteral("visible"), !rows.isEmpty()}};
}

int AnnualState::groupedCount() const {
  return annualTransactionGroups_.value(qstr(kDeduplicated)).toList().size() +
         annualTransactionGroups_.value(qstr(kSimilar)).toList().size() +
         annualTransactionGroups_.value(qstr(kDivergent)).toList().size() +
         annualTransactionGroups_.value(qstr(kWorkspaceOnly)).toList().size();
}

int AnnualState::workspaceRevision() const {
  return workspace_ ? workspace_->dataRevision() : -1;
}

void AnnualState::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (!workspace_) {
    return;
  }
  connect(workspace_, &WorkspaceFacade::selectedAnnualIdChanged, this,
          &AnnualState::refreshFromSelection);
  connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
          &AnnualState::refreshFromSelection);
}

void AnnualState::emitChanged() { emit changed(); }

} // namespace ui
