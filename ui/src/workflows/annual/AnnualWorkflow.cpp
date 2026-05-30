/**
 * @file ui/src/workflows/annual/AnnualWorkflow.cpp
 * @brief Implements the UI workflow for annual result computation.
 */

#include "ui/workflows/annual/AnnualWorkflow.h"

#include <utility>

#include "core/application/annual/AnnualService.h"
#include "ui/adapters/core/AnnualRequestMapper.h"
#include "ui/adapters/core/AnnualResultMapper.h"

namespace ui {

AnnualWorkflow::AnnualWorkflow(
    StateSnapshotProvider stateSnapshotProvider,
    std::shared_ptr<core::application::annual::AnnualService> annualService,
    QObject *parent)
    : QObject(parent),
      stateSnapshotProvider_(std::move(stateSnapshotProvider)),
      annualService_(std::move(annualService)) {}

QVariantMap AnnualWorkflow::computeAnnual(const QString &annualId) const {
  if (!annualService_ || annualId.trimmed().isEmpty()) {
    return {};
  }
  try {
    return annual::toPayload(annualService_->runAnnual(
        stateSnapshot(), annual::toRequest(annualId.trimmed())));
  } catch (...) {
    return {};
  }
}

QVariantMap
AnnualWorkflow::computeAnnualPreview(const QString &annualId,
                                     const QStringList &analysisIds,
                                     int year) const {
  if (!annualService_) {
    return {};
  }
  try {
    const QString previewId = QStringLiteral("__annual_preview__");
    const auto snapshot = annual::withPreviewAnnual(
        stateSnapshot(), annualId.trimmed().isEmpty() ? previewId
                                                      : annualId.trimmed(),
        analysisIds, year);
    return annual::toPayload(
        annualService_->runAnnual(snapshot, annual::toRequest(annualId.trimmed().isEmpty()
                                                                  ? previewId
                                                                  : annualId.trimmed())));
  } catch (...) {
    return {};
  }
}

core::ports::workspace::WorkspaceSnapshot AnnualWorkflow::stateSnapshot() const {
  return stateSnapshotProvider_
             ? stateSnapshotProvider_()
             : core::ports::workspace::WorkspaceSnapshot{};
}

} // namespace ui
