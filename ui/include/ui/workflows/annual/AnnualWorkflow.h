/**
 * @file ui/include/ui/workflows/annual/AnnualWorkflow.h
 * @brief Declares the UI workflow for annual result computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace core::application::annual {
class AnnualService;
}

namespace ui {

/**
 * @brief Exposes annual use-case computation to the UI boundary.
 */
class AnnualWorkflow : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(AnnualWorkflow)
  QML_UNCREATABLE("AnnualWorkflow is provided by the application context")

public:
  using StateSnapshotProvider =
      std::function<core::ports::workspace::WorkspaceSnapshot()>;

  explicit AnnualWorkflow(
      StateSnapshotProvider stateSnapshotProvider,
      std::shared_ptr<core::application::annual::AnnualService> annualService,
      QObject *parent = nullptr);

  Q_INVOKABLE QVariantMap computeAnnual(const QString &annualId) const;
  Q_INVOKABLE QVariantMap computeAnnualPreview(const QString &annualId,
                                               const QStringList &analysisIds,
                                               int year) const;

private:
  core::ports::workspace::WorkspaceSnapshot stateSnapshot() const;

  StateSnapshotProvider stateSnapshotProvider_;
  std::shared_ptr<core::application::annual::AnnualService> annualService_;
};

} // namespace ui
