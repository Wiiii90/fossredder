#pragma once

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

class AnalysisEngine;
struct AppState;
class AppStateController;

namespace ui {

class AnalysisController : public QObject {
    Q_OBJECT
public:
    using StateSnapshotProvider = std::function<std::shared_ptr<const AppState>()>;

    explicit AnalysisController(AppStateController* core,
                                StateSnapshotProvider stateSnapshotProvider,
                                const AnalysisEngine* analysisEngine,
                                QObject* parent = nullptr);

    Q_INVOKABLE QString addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;
    Q_INVOKABLE QStringList getContractTypes() const;

private:
    std::shared_ptr<const AppState> stateSnapshot() const;

    AppStateController* core_ = nullptr;
    StateSnapshotProvider stateSnapshotProvider_;
    const AnalysisEngine* analysisEngine_ = nullptr;
};

}
