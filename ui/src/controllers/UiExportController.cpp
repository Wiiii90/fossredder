#include "ui/controllers/UiExportController.h"
#include "core/controllers/ExportController.h"
#include "core/controllers/CsvController.h"
#include "core/controllers/XlsxController.h"

#include <memory>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>

UiExportController::UiExportController(AppStateController* core, QObject* parent)
    : QObject(parent), core_(core)
{
    connect(&exportWatcher_, &QFutureWatcher<bool>::finished, this, &UiExportController::onExportFinished);
}



void UiExportController::exportData(int format, const QString& path, bool includeFormulas, const QString& locale)
{
    if (!core_) {
        qDebug() << "UiExportController::exportData called but core_ is null";
        return;
    }
    if (isRunning_) return;
    isRunning_ = true;
    emit stateChanged();

    // Debug: report snapshot sizes
    if (core_ && core_->state().properties.empty() && core_->state().transactions.empty()) {
        qDebug() << "Export: AppState appears empty: properties=" << core_->state().properties.size()
                 << " contracts=" << core_->state().contracts.size()
                 << " transactions=" << core_->state().transactions.size();
    } else {
        qDebug() << "Export: preparing export: properties=" << core_->state().properties.size()
                 << " contracts=" << core_->state().contracts.size()
                 << " transactions=" << core_->state().transactions.size();
    }

    // Prepare controllers
    auto csv = std::make_shared<core::controllers::exporting::CsvController>();
    auto xlsx = std::make_shared<core::controllers::exporting::XlsxController>();
    core::controllers::exporting::ExportController exporter(xlsx, csv);

    core::controllers::exporting::ExportOptions opts;
    opts.outputPath = path.toStdString();
    opts.includeFormulas = includeFormulas;
    opts.locale = locale.toStdString();
    opts.state = &core_->state();
    opts.format = (format == 0) ? core::controllers::exporting::ExportOptions::Format::Csv : core::controllers::exporting::ExportOptions::Format::Xlsx;
    // remember parameters
    lastPath_ = path;
    lastFormat_ = format;
    lastIncludeFormulas_ = includeFormulas;
    lastLocale_ = locale;

    exportFuture_ = QtConcurrent::run([exporter, opts]() mutable {
        return exporter.exportData(opts);
    });
    exportWatcher_.setFuture(exportFuture_);
}

void UiExportController::onExportFinished()
{
    bool success = exportFuture_.result();

    // Post-check for XLSX: some environments may write CSV content but user selected .xlsx
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
                // maybe a CSV was written to the .xlsx path (small text file). If so, rename to .csv
                QFile f2(lastPath_);
                if (f2.open(QIODevice::ReadOnly)) {
                    QByteArray content = f2.read(1024);
                    f2.close();
                    const QString s = QString::fromUtf8(content);
                    if (s.contains("Data") || s.contains("Property") || s.contains(';') ) {
                        QString newPath = lastPath_;
                        if (newPath.toLower().endsWith(".xlsx")) newPath = newPath.left(newPath.size()-5) + ".csv";
                        // try move
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
