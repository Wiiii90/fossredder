/**
 * @file ui/include/ui/controllers/AnalysisController.h
 * @brief Declares the UI controller for analysis creation and computation.
 */

#pragma once

#include <functional>
#include <memory>

#include <QObject>
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
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;
    Q_INVOKABLE QStringList getContractTypes() const;

private:
    std::shared_ptr<const core::domain::AppState> stateSnapshot() const;

    core::application::AppStateFacade* core_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    std::shared_ptr<core::application::AnalysisService> analysisService_;
};

} // namespace ui
