#include "ui/models/AnalysisList.h"

#include "core/application/AnalysisRequestComposer.h"
#include "ui/config/Defaults.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/payload/PayloadKeys.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporterRegistry.h"

#include <QVariant>

#include <exception>

namespace ui {

QString AnalysisList::adjustmentsJsonFor(const Analysis &analysis) {
  try {
    return QString::fromStdString(
        core::application::AnalysisRequestComposer::serializeAdjustments(analysis.adjustments));
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::ExceptionError,
        observability::origins::model::analysisList::kAdjustmentsJson,
        std::current_exception());
  }
  return ui::config::kJsonEmptyObject;
}

void AnalysisList::rebuildAdjustmentsCache() {
  adjustmentsJsonById_.clear();
  adjustmentsJsonById_.reserve(static_cast<int>(analyses().size()));
  for (const auto &analysis : analyses()) {
    if (!analysis)
      continue;
    updateAdjustmentsCache(*analysis);
  }
}

void AnalysisList::updateAdjustmentsCache(const Analysis &analysis) {
  adjustmentsJsonById_.insert(QString::fromStdString(analysis.id),
                              adjustmentsJsonFor(analysis));
}

AnalysisList::AnalysisList(QObject *parent) : Base(parent) {}

QVariant AnalysisList::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  const auto &a = itemAtRow(index.row());
  if (!a)
    return {};

  const QString adjustmentsJson = adjustmentsJsonById_.value(
      QString::fromStdString(a->id), ui::config::kJsonEmptyObject);

  switch (role) {
  case IdRole:
    return QString::fromStdString(a->id);
  case NameRole:
    return QString::fromStdString(a->name);
  case TypeRole:
    return QString::fromStdString(a->type);
  case ConfigRole:
    return QString::fromStdString(a->configJson);
  case FilterRole:
    return QString::fromStdString(a->filterSpec);
  case AdjustmentsRole:
    return adjustmentsJson;
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
  return roles;
}

int AnalysisList::addAnalysis(const QString &name, const QString &type) {
  auto a = std::make_shared<Analysis>();
  a->name = strings::toStdString(name);
  a->type = strings::toStdString(type);
  updateAdjustmentsCache(*a);
  return appendItem(std::move(a));
}

void AnalysisList::setAnalyses(
    std::vector<std::shared_ptr<Analysis>> analyses) {
  setItems(std::move(analyses));
  rebuildAdjustmentsCache();
}

void AnalysisList::removeAt(int row) {
  const auto &analysis = itemAtRow(row);
  if (analysis)
    adjustmentsJsonById_.remove(QString::fromStdString(analysis->id));
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

  a->name = strings::toStdString(name);
  a->type = strings::toStdString(type);
  a->configJson = strings::toStdString(configJson);
  a->filterSpec = strings::toStdString(filterSpec);
  updateAdjustmentsCache(*a);
  const QModelIndex idx = index(row);
  emit dataChanged(
      idx, idx, {NameRole, TypeRole, ConfigRole, FilterRole, AdjustmentsRole});
  return true;
}

void AnalysisList::setAdjustmentsById(const QString &id, const QString &json) {
  const int row = findRowById(id);
  if (row < 0)
    return;

  const auto &a = itemAtRow(row);
  if (!a)
    return;

  a->adjustments.clear();
  try {
    const auto parsed = core::application::AnalysisRequestComposer::parseAdjustmentsJson(json.toStdString());
    if (!parsed.valid) {
      core::errors::report(
          core::errors::ErrorSeverity::Warning,
          core::errors::codes::GenericError,
          observability::origins::model::analysisList::kAdjustmentsParse,
          parsed.error.empty() ? std::string("Failed to parse analysis adjustments") : parsed.error);
    } else {
      a->adjustments = std::move(parsed.adjustments);
    }
  } catch (...) {
    core::errors::reportException(
        core::errors::ErrorSeverity::Warning,
        core::errors::codes::ExceptionError,
        observability::origins::model::analysisList::kSetAdjustments,
        std::current_exception());
  }
  updateAdjustmentsCache(*a);
  const QModelIndex idx = index(row);
  emit dataChanged(idx, idx, {AdjustmentsRole});
}

} // namespace ui
