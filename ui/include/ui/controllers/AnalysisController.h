/**
 * @file ui/include/ui/controllers/AnalysisController.h
 * @brief Declares the UI controller for analysis creation and computation.
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

namespace core::application { class AppStateFacade; class AnalysisService; }
namespace core::domain { struct AppState; }

namespace ui {

/**
 * @brief Exposes analysis actions and computations to QML.
 */
class AnalysisController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AnalysisController)
    QML_UNCREATABLE("AnalysisController is provided by the application context")
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::AppState>()>;

    /** @brief Create an analysis controller with access to the facade and analysis service. */
    explicit AnalysisController(core::application::AppStateFacade* core,
                                StateSnapshotProvider stateSnapshotProvider,
                                std::shared_ptr<core::application::AnalysisService> analysisService,
                                QObject* parent = nullptr);

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
    Q_INVOKABLE QString createAnalysis(const QString& name,
                                       const QString& type,
                                       const QString& configJson,
                                       const QString& filterSpec);

    /** @brief Update an existing analysis from serialized inputs.
     *  @param id Analysis identifier
     *  @param name Analysis name
     *  @param type Analysis type
     *  @param configJson JSON configuration
     *  @param filterSpec Filter specification string
     */
    Q_INVOKABLE void updateAnalysis(const QString& id,
                                    const QString& name,
                                    const QString& type,
                                    const QString& configJson,
                                    const QString& filterSpec);

    /** @brief Delete an analysis by identifier.
     *  @param id Analysis identifier
     */
    Q_INVOKABLE void deleteAnalysis(const QString& id);

    /** @brief Build an analysis config JSON string from UI parameters.
     *  @param type Analysis type
     *  @param plotType Plot type
     *  @param plotMeasure Plot measure
     *  @param propertyIds Property identifiers
     *  @param contractTypes Contract types
     *  @param taxPercent Tax percent
     *  @return JSON string
     */
    Q_INVOKABLE QString analysisConfigJson(const QString& type,
                                          const QString& plotType,
                                          const QString& plotMeasure,
                                          const QStringList& propertyIds,
                                          const QStringList& contractTypes,
                                          double taxPercent) const;

    /** @brief Build an analysis filter specification from the selected date range.
     *  @param dateFrom Start date string
     *  @param dateTo End date string
     *  @return Filter specification string
     */
    Q_INVOKABLE QString analysisFilterSpec(const QString& dateFrom, const QString& dateTo) const;

    /** @brief Build the JSON adjustments payload for selected transactions.
     *  @param transactions List of transactions
     *  @param selectedTransactionIds Selected transaction identifiers
     *  @param taxPercent Tax percent
     *  @return JSON string
     */
    Q_INVOKABLE QString analysisAdjustmentsJson(const QVariantList& transactions,
                                                const QVariantList& selectedTransactionIds,
                                                double taxPercent) const;

    /** @brief Compute an analysis using the configured analysis service.
     *  @param analysisId Analysis identifier
     *  @param filterSpec Filter specification string
     *  @return Serialized analysis result as QVariantMap
     */
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;

    /** @brief Return the currently configured contract types. */
    Q_INVOKABLE QStringList contractTypes() const;

private:
    std::shared_ptr<const core::domain::AppState> stateSnapshot() const;

    core::application::AppStateFacade* core_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    std::shared_ptr<core::application::AnalysisService> analysisService_;
};

} // namespace ui
