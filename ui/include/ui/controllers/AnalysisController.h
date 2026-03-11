#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace core::application { class AnalysisService; }
namespace core::analysis { class AnalysisEngine; }
namespace core::domain { struct AppState; }
namespace core::controllers { class AppStateController; }

namespace ui {

class AnalysisController : public QObject {
    Q_OBJECT
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const core::domain::AppState>()>;

    explicit AnalysisController(core::controllers::AppStateController* core,
                                StateSnapshotProvider stateSnapshotProvider,
                                const core::application::AnalysisService* analysisService,
                                QObject* parent = nullptr);

    Q_INVOKABLE QString addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;
    Q_INVOKABLE QStringList getContractTypes() const;

private:
    std::shared_ptr<const core::domain::AppState> stateSnapshot() const;

    core::controllers::AppStateController* core_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    const core::application::AnalysisService* analysisService_ = nullptr;
};

}
