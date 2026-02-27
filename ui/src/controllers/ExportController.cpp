#include "ui/controllers/ExportController.h"

#include "core/controllers/CsvController.h"
#include "core/controllers/ExportController.h"
#include "core/controllers/XlsxController.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QFile>

namespace ui {

ExportController::ExportController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
    connect(&exportWatcher_, &QFutureWatcher<bool>::finished, this, &ExportController::onExportFinished);
}

void ExportController::exportData(int format, const QString& path, bool includeFormulas, const QString& locale)
{
    if (!core_) return;
    if (isRunning_) return;

    isRunning_ = true;
    emit stateChanged();

    auto csv = std::make_shared<core::controllers::exporting::CsvController>();
    auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
    core::controllers::exporting::ExportController exporter(xlsx, csv);

    core::controllers::exporting::ExportOptions opts;
    opts.outputPath = path.toStdString();
    opts.includeFormulas = includeFormulas;
    opts.locale = locale.toStdString();
    opts.state = &core_->state();
    opts.format = (format == 0) ? core::controllers::exporting::ExportOptions::Format::Csv : core::controllers::exporting::ExportOptions::Format::Xlsx;

    lastPath_ = path;
    lastFormat_ = format;
    lastIncludeFormulas_ = includeFormulas;
    lastLocale_ = locale;

    exportFuture_ = QtConcurrent::run([exporter, opts]() mutable {
        return exporter.exportData(opts);
    });
    exportWatcher_.setFuture(exportFuture_);
}

void ExportController::onExportFinished()
{
    bool success = exportFuture_.result();

    if (!lastPath_.isEmpty() && lastFormat_ == 1) {
        QFile f(lastPath_);
        if (f.exists()) {
            bool looksLikeZip = false;
            if (f.open(QIODevice::ReadOnly)) {
                QByteArray hdr = f.read(4);
                f.close();
                if (hdr.size() >= 2 && hdr[0] == 'P' && hdr[1] == 'K') looksLikeZip = true;
            }
            if (!looksLikeZip) {
                QFile f2(lastPath_);
                if (f2.open(QIODevice::ReadOnly)) {
                    QByteArray content = f2.read(1024);
                    f2.close();
                    const QString s = QString::fromUtf8(content);
                    if (s.contains("Data") || s.contains("Property") || s.contains(';')) {
                        QString newPath = lastPath_;
                        if (newPath.toLower().endsWith(".xlsx")) newPath = newPath.left(newPath.size() - 5) + ".csv";
                        QFile::remove(newPath);
                        if (QFile::rename(lastPath_, newPath)) {
                            lastPath_ = newPath;
                            success = true;
                        }
                    }
                }
            }
        }
    }

    isRunning_ = false;
    emit stateChanged();
    emit exportFinished(success);
}

}
