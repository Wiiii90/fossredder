#pragma once

#include <QObject>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>

#include "core/controllers/AppStateController.h"

class UiExportController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
public:
    explicit UiExportController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE void exportData(int format, const QString& path, bool includeFormulas = true, const QString& locale = QString());

    bool isRunning() const noexcept { return isRunning_; }

signals:
    void stateChanged();
    void exportFinished(bool success);

private slots:
    void onExportFinished();

private:
    AppStateController* core_ = nullptr;
    QFuture<bool> exportFuture_;
    QFutureWatcher<bool> exportWatcher_;
    bool isRunning_ = false;
    // store last export parameters for post-checks / fallback
    QString lastPath_;
    int lastFormat_ = 0;
    bool lastIncludeFormulas_ = true;
    QString lastLocale_;
};
