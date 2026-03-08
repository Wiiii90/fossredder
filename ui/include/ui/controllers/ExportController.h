#pragma once

#include <QObject>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>
#include <memory>

#include "core/controllers/AppStateController.h"
#include "core/export/ExportOptions.h"

namespace ui::exporting { class ExportRunner; }

namespace ui {

class ExportController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY stateChanged)
public:
    explicit ExportController(AppStateController* core,
                              std::shared_ptr<ui::exporting::ExportRunner> runner,
                              QObject* parent = nullptr);

    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());

    bool isRunning() const noexcept { return isRunning_; }
    QString lastError() const { return lastError_; }

signals:
    void stateChanged();
    void exportFinished(bool success);
    void exportFailed(const QString& error);

private slots:
    void onExportFinished();

private:
    AppStateController* core_ = nullptr;
    std::shared_ptr<ui::exporting::ExportRunner> runner_;
    QFuture<core::controllers::exporting::ExportOptions> exportFuture_;
    QFutureWatcher<core::controllers::exporting::ExportOptions> exportWatcher_;
    bool isRunning_ = false;
    QString lastError_;
};

}
