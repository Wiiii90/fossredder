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

    /** @brief Create an analysis controller with access to the facade and analysis service. */
    explicit AnalysisController(core::application::AppStateFacade* core,
                                StateSnapshotProvider stateSnapshotProvider,
                                std::shared_ptr<core::application::AnalysisService> analysisService,
                                QObject* parent = nullptr);

    /** @brief Return a single analysis by identifier. */
    Q_INVOKABLE QVariantMap analysis(const QString& id) const;

    /** @brief Return all analyses. */
    Q_INVOKABLE QVariantList analyses() const;

    /** @brief Create a new analysis from serialized inputs. */
    Q_INVOKABLE QString createAnalysis(const QString& name,
                                       const QString& type,
                                       const QString& configJson,
                                       const QString& filterSpec);

    /** @brief Update an existing analysis from serialized inputs. */
    Q_INVOKABLE void updateAnalysis(const QString& id,
                                    const QString& name,
                                    const QString& type,
                                    const QString& configJson,
                                    const QString& filterSpec);

    /** @brief Delete an analysis by identifier. */
    Q_INVOKABLE void deleteAnalysis(const QString& id);

    /** @brief Build an analysis config JSON string from UI parameters. */
    Q_INVOKABLE QString analysisConfigJson(const QString& type,
                                          const QString& plotType,
                                          const QString& plotMeasure,
                                          const QStringList& propertyIds,
                                          const QStringList& contractTypes,
                                          double taxPercent) const;

    /** @brief Build an analysis filter specification from the selected date range. */
    Q_INVOKABLE QString analysisFilterSpec(const QString& dateFrom, const QString& dateTo) const;

    /** @brief Build the JSON adjustments payload for selected transactions. */
    Q_INVOKABLE QString analysisAdjustmentsJson(const QVariantList& transactions,
                                                const QVariantList& selectedTransactionIds,
                                                double taxPercent) const;

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
