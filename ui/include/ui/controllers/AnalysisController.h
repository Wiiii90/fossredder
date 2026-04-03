/**
 * @file ui/include/ui/controllers/AnalysisController.h
 * @brief Declares the UI controller for analysis creation and computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QVariantList>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace core::application { class AppStateFacade; class AnalysisService; }
namespace core::domain { struct AppState; }

namespace ui {

/**
 * @brief Exposes analysis actions and computations to QML.
 */
class AnalysisController : public QObject {
    Q_OBJECT
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::AppState>()>;

    explicit AnalysisController(core::application::AppStateFacade* core,
                                StateSnapshotProvider stateSnapshotProvider,
                                std::shared_ptr<core::application::AnalysisService> analysisService,
                                QObject* parent = nullptr);

    Q_INVOKABLE QString addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    /** @brief Builds and creates an analysis definition from raw UI inputs. */
    Q_INVOKABLE QString createAnalysisFromUi(const QString& name,
                                             const QString& type,
                                             const QString& plotType,
                                             const QString& plotMeasure,
                                             const QStringList& propertyIds,
                                             const QStringList& contractTypes,
                                             const QString& dateFrom,
                                             const QString& dateTo,
                                             double taxPercent = 0.0);
    /** @brief Creates an analysis from UI inputs and computes the initial result in one call. */
    Q_INVOKABLE QVariantMap createAnalysisFromUiAndCompute(const QString& name,
                                                           const QString& type,
                                                           const QString& plotType,
                                                           const QString& plotMeasure,
                                                           const QStringList& propertyIds,
                                                           const QStringList& contractTypes,
                                                           const QString& dateFrom,
                                                           const QString& dateTo,
                                                           double taxPercent = 0.0);
    /** @brief Maps a strategy index and executes create+compute in one call. */
    Q_INVOKABLE QVariantMap createAnalysisFromStrategyAndCompute(const QString& name,
                                                                 int strategyIndex,
                                                                 const QString& plotType,
                                                                 const QString& plotMeasure,
                                                                 const QStringList& propertyIds,
                                                                 const QStringList& contractTypes,
                                                                 const QString& dateFrom,
                                                                 const QString& dateTo,
                                                                 double taxPercent = 0.0);
    /** @brief Composes the analysis filter clause for the supplied date bounds. */
    Q_INVOKABLE QString buildFilterSpec(const QString& dateFrom, const QString& dateTo) const;
    /** @brief Builds the JSON adjustments payload for selected transactions. */
    Q_INVOKABLE QString buildTaxAdjustmentsJson(const QVariantList& transactions,
                                                const QVariantList& selectedTransactionIds,
                                                double taxPercent) const;
    /** @brief Builds adjustments and recomputes the analysis result in one call. */
    Q_INVOKABLE QVariantMap applyTaxAdjustmentsAndRecompute(const QString& analysisId,
                                                            const QString& filterSpec,
                                                            const QVariantList& transactions,
                                                            const QVariantList& selectedTransactionIds,
                                                            double taxPercent) const;
    /** @brief Parses a percent string and applies tax adjustments in one call. */
    Q_INVOKABLE QVariantMap applyTaxAdjustmentsAndRecomputeFromText(const QString& analysisId,
                                                                    const QString& filterSpec,
                                                                    const QVariantList& transactions,
                                                                    const QVariantList& selectedTransactionIds,
                                                                    const QString& taxPercentText) const;
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;
    Q_INVOKABLE QStringList getContractTypes() const;

private:
    std::shared_ptr<const core::domain::AppState> stateSnapshot() const;

    core::application::AppStateFacade* core_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    std::shared_ptr<core::application::AnalysisService> analysisService_;
};

} // namespace ui
