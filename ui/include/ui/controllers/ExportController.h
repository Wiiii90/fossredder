#pragma once

#include <QObject>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>

#include "core/controllers/AppStateController.h"
#include "core/export/ExportOptions.h"

namespace ui {

class ExportController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
public:
    explicit ExportController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());

    bool isRunning() const noexcept { return isRunning_; }

signals:
    void stateChanged();
    void exportFinished(bool success);

private slots:
    void onExportFinished();

private:
    AppStateController* core_ = nullptr;
    QFuture<core::controllers::exporting::ExportOptions> exportFuture_;
    QFutureWatcher<core::controllers::exporting::ExportOptions> exportWatcher_;
    bool isRunning_ = false;
};

}
