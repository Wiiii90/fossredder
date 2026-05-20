/**
 * @file ui/include/ui/workflows/analysis/AnalysisWorkflow.h
 * @brief Declares the UI workflow for analysis creation and computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QStringList>
#include <qqmlintegration.h>

#include "core/application/analysis/AnalysisRequest.h"
#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

namespace core::application::analysis { class AnalysisService; }
namespace core::ports::analysis_image_renderer { class IAnalysisImageRenderer; }
namespace core::ports::presenters { class IAnalysisPresenter; }

namespace ui {

/**
 * @brief Exposes analysis workflow actions and computations to QML.
 */
class AnalysisWorkflow : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AnalysisWorkflow)
    QML_UNCREATABLE("AnalysisWorkflow is provided by the application context")
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::catalog::WorkspaceCatalog>()>;

    /** @brief Create an analysis workflow with access to the facade and analysis service. */
    explicit AnalysisWorkflow(core::ports::workspace::IWorkspaceWriter* core,
                              StateSnapshotProvider stateSnapshotProvider,
                              std::shared_ptr<core::application::analysis::AnalysisService> analysisService,
                              std::shared_ptr<core::ports::presenters::IAnalysisPresenter> analysisPresenter = {},
                              QObject* parent = nullptr,
                              std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer = {});

    /** @brief Return a single analysis by identifier.
     *  @param id Analysis identifier
     *  @return Serialized analysis payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap analysis(const QString& id) const;

    /** @brief Return all analyses.
     *  @return List of serialized analyses as QVariantList
     */
    Q_INVOKABLE QVariantList analyses() const;

    /** @brief Create a new analysis from serialized inputs.
     *  @param name Analysis name
     *  @param type Analysis type
     *  @param configJson JSON configuration
     *  @param filterSpec Filter specification string
     *  @return Identifier of the created analysis
     */
    Q_INVOKABLE QString createAnalysis(const QString& name, const QString& type,
                                       const QString& configJson, const QString& filterSpec,
                                       const QString& exportFormat, bool includeCalcAdjustments,
                                       const QString& exportStateJson, const QString& snapshotTransactionsJson,
                                       const QString& adjustmentsJson = QStringLiteral("{}"));

    /** @brief Update an existing analysis from serialized inputs.
     *  @param id Analysis identifier
     *  @param name Analysis name
     *  @param type Analysis type
     *  @param configJson JSON configuration
     *  @param filterSpec Filter specification string
     */
    Q_INVOKABLE void updateAnalysis(const QString& id, const QString& name,
                                    const QString& type, const QString& configJson,
                                    const QString& filterSpec, const QString& exportFormat,
                                    bool includeCalcAdjustments, const QString& exportStateJson,
                                    const QString& snapshotTransactionsJson,
                                    const QString& adjustmentsJson = QStringLiteral("{}"));

    /** @brief Delete an analysis by identifier.
     *  @param id Analysis identifier
     */
    Q_INVOKABLE void deleteAnalysis(const QString& id);

    /** @brief Build an analysis request DTO from UI inputs.
     *  @param analysisId Analysis identifier
     *  @param filterSpecification Filter specification string
     *  @return Core analysis request DTO
     */
    Q_INVOKABLE core::application::analysis::AnalysisRequest analysisRequest(const QString& analysisId,
                                                                             const QString& filterSpecification) const;

    /** @brief Build a canonical analysis filter specification from UI inputs. */
    Q_INVOKABLE QString analysisFilterSpec(const QString& dateField,
                                           const QString& dateMode,
                                           const QString& year,
                                           const QString& dateFrom,
                                           const QString& dateTo,
                                           const QStringList& propertyIds,
                                           const QStringList& contractTypes,
                                           const QString& allocatableMode) const;
    Q_INVOKABLE QVariantMap parseAnalysisFilterSpec(const QString& filterSpec) const;

    /** @brief Build the core-compatible analysis configuration JSON from UI inputs. */
    Q_INVOKABLE QString analysisConfigJson(const QString& type,
                                           const QString& plotType,
                                           const QString& plotMeasure,
                                           const QStringList& propertyIds,
                                           const QStringList& contractTypes,
                                           double taxPercent) const;

    /** @brief Build persisted adjustment amounts for selected preview transactions. */
    Q_INVOKABLE QString analysisAdjustmentsJson(const QVariantList& transactions,
                                                const QStringList& selectedTransactionIds,
                                                double taxPercent) const;

    /** @brief Compute an analysis using the configured analysis service.
     *  @param request Analysis request DTO
     *  @return Serialized analysis result as QVariantMap
     */
    Q_INVOKABLE QVariantMap computeAnalysis(const core::application::analysis::AnalysisRequest& request) const;

    /** @brief Compute an analysis from QML-friendly string inputs. */
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId,
                                            const QString& filterSpecification) const;

    /** @brief Compute an analysis preview using transient UI adjustment settings. */
    Q_INVOKABLE QVariantMap computeAnalysisPreview(const QString& analysisId,
                                                   const QString& filterSpecification,
                                                   bool includeCalcAdjustments,
                                                   const QString& adjustmentsJson) const;

    /** @brief Return filtered transaction preview data for the analysis builder. */
    Q_INVOKABLE QVariantMap previewTransactions(const QString& filterSpec) const;

    /** @brief Return the currently configured contract types. */
    Q_INVOKABLE QStringList contractTypes() const;

private:
    std::shared_ptr<const core::domain::catalog::WorkspaceCatalog> stateSnapshot() const;
    QVariantMap payloadWithRenderedPreview(const core::application::analysis::AnalysisRequest& request,
                                           core::application::analysis::AnalysisResult result) const;

    core::ports::workspace::IWorkspaceWriter* core_ = nullptr;
    core::ports::workspace::IWorkspaceReader* reader_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    std::shared_ptr<core::application::analysis::AnalysisService> analysisService_;
    std::shared_ptr<core::ports::presenters::IAnalysisPresenter> analysisPresenter_;
    std::shared_ptr<core::ports::analysis_image_renderer::IAnalysisImageRenderer> imageRenderer_;
};
} // namespace ui
