/**
 * @file ui/src/models/AnalysisList.cpp
 * @brief Implementation of the UI AnalysisList component.
 */

#include "ui/viewmodels/reporting/AnalysisListModel.h"

#include "core/constants/json.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/shared/util/StringConversions.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include <exception>

namespace ui {

QString AnalysisList::serializeAdjustmentsJson(const Analysis &analysis) {
  try {
    QJsonObject obj;
    for (const auto &[id, amount] : analysis.adjustments()) {
      obj.insert(QString::fromStdString(id), amount);
    }
    return QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::ExceptionError,
        observability::origins::model::analysisList::kAdjustmentsJson,
        std::current_exception());
  }
  return QString::fromLatin1(core::constants::json::kEmptyObject.data());
}

void AnalysisList::refreshAdjustmentsCache() {
  adjustmentsJsonById_.clear();
  adjustmentsJsonById_.reserve(static_cast<int>(analyses().size()));
  for (const auto &analysis : analyses()) {
    if (!analysis)
      continue;
    refreshAdjustmentsCacheEntry(*analysis);
  }
}

void AnalysisList::refreshAdjustmentsCacheEntry(const Analysis &analysis) {
  adjustmentsJsonById_.insert(QString::fromStdString(analysis.id()),
                              serializeAdjustmentsJson(analysis));
}

AnalysisList::AnalysisList(QObject *parent) : Base(parent) {}

QVariant AnalysisList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &a = itemAtRow(index.row());
  if (!a)
    return {};

  const QString adjustmentsJson = adjustmentsJsonById_.value(
      QString::fromStdString(a->id()),
      QString::fromLatin1(core::constants::json::kEmptyObject.data()));

  switch (role) {
  case IdRole:
    return QString::fromStdString(a->id());
  case NameRole:
    return QString::fromStdString(a->name());
  case TypeRole:
    return QString::fromStdString(a->type());
  case ConfigRole:
    return QString::fromStdString(a->configJson());
  case FilterRole:
    return QString::fromStdString(a->filterSpec());
  case AdjustmentsRole:
    return adjustmentsJson;
  case ExportFormatRole:
    return QString::fromStdString(a->exportFormat());
  case IncludeCalcAdjustmentsRole:
    return a->includeCalculationAdjustments();
  case ExportStateRole:
    return QString::fromStdString(a->exportStateJson());
  case SnapshotTransactionsRole:
    return QString::fromStdString(a->snapshotTransactionsJson());
  case CreatedAtRole:
    return QString::fromStdString(a->createdAt());
  case UpdatedAtRole:
    return QString::fromStdString(a->updatedAt());
  default:
    return {};
  }
}

QHash<int, QByteArray> AnalysisList::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = ui::payload::keys::common::kId.toUtf8();
  roles[NameRole] = ui::payload::keys::common::kName.toUtf8();
  roles[TypeRole] = ui::payload::keys::common::kType.toUtf8();
  roles[ConfigRole] = ui::payload::keys::analysis::kConfig.toUtf8();
  roles[FilterRole] = ui::payload::keys::analysis::kFilter.toUtf8();
  roles[AdjustmentsRole] = ui::payload::keys::analysis::kAdjustments.toUtf8();
  roles[ExportFormatRole] = ui::payload::keys::analysis::kExportFormat.toUtf8();
  roles[IncludeCalcAdjustmentsRole] = ui::payload::keys::analysis::kIncludeCalcAdjustments.toUtf8();
  roles[ExportStateRole] = ui::payload::keys::analysis::kExportState.toUtf8();
  roles[SnapshotTransactionsRole] = ui::payload::keys::analysis::kSnapshotTransactions.toUtf8();
  roles[CreatedAtRole] = ui::payload::keys::analysis::kCreatedAt.toUtf8();
  roles[UpdatedAtRole] = ui::payload::keys::analysis::kUpdatedAt.toUtf8();
  return roles;
}

int AnalysisList::addAnalysis(const QString &name, const QString &type) {
  auto a = std::make_shared<Analysis>();
  a->rename(strings::toStdString(name));
  a->setType(strings::toStdString(type));
  refreshAdjustmentsCacheEntry(*a);
  return appendItem(std::move(a));
}

void AnalysisList::setAnalyses(
    std::vector<std::shared_ptr<Analysis>> analyses) {
  setItems(std::move(analyses));
  refreshAdjustmentsCache();
}

void AnalysisList::removeAt(int row) {
  const auto &analysis = itemAtRow(row);
  if (analysis)
    adjustmentsJsonById_.remove(QString::fromStdString(analysis->id()));
  removeItemAt(row);
}

bool AnalysisList::updateAnalysisById(const QString &id, const QString &name,
                                      const QString &type,
                                      const QString &configJson,
                                      const QString &filterSpec) {
  const int row = findRowById(id);
  if (row < 0)
    return false;

  const auto &a = itemAtRow(row);
  if (!a)
    return false;

  a->rename(strings::toStdString(name));
  a->setType(strings::toStdString(type));
  a->setConfigJson(strings::toStdString(configJson));
  a->setFilterSpec(strings::toStdString(filterSpec));
  refreshAdjustmentsCacheEntry(*a);
  const QModelIndex idx = index(row);
  emit dataChanged(
      idx, idx, {NameRole, TypeRole, ConfigRole, FilterRole, AdjustmentsRole});
  return true;
}

void AnalysisList::setAdjustmentsById(const QString &id, const QString &adjustmentsJson) {
  const int row = findRowById(id);
  if (row < 0)
    return;

  const auto &a = itemAtRow(row);
  if (!a)
    return;

  a->clearAdjustments();
  try {
    const QJsonDocument doc =
        QJsonDocument::fromJson(adjustmentsJson.toUtf8());
    if (!doc.isObject()) {
      core::errors::report(
          core::errors::ErrorSeverity::Warning,
          core::errors::codes::GenericError,
          observability::origins::model::analysisList::kAdjustmentsParse,
          std::string("Failed to parse analysis adjustments"));
    } else {
      const QJsonObject obj = doc.object();
      for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        if (!it.value().isDouble())
          continue;
        a->setAdjustment(it.key().toStdString(), it.value().toDouble());
      }
    }
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::ExceptionError,
        observability::origins::model::analysisList::kSetAdjustments,
        std::current_exception());
  }
  refreshAdjustmentsCacheEntry(*a);
  const QModelIndex idx = index(row);
  emit dataChanged(idx, idx, {AdjustmentsRole});
}

} // namespace ui
