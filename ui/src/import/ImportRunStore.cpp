#include "ui/import/ImportRunStore.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#include "ui/config/Defaults.h"

namespace ui::importing {

ImportRunInfo createImportRunInfo()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir rootDir(base);
    rootDir.mkpath(".");

    const QString timestamp = QDateTime::currentDateTimeUtc().toString(ui::config::kImportRunTimestampFormat);

    int suffix = ui::config::kImportRunFirstSuffix;
    while (true) {
        const QString runName = ui::config::kImportRunNamePattern.arg(timestamp).arg(suffix);
        if (!rootDir.exists(runName)) {
            rootDir.mkpath(runName);
            return {rootDir.filePath(runName), timestamp};
        }
        ++suffix;
    }
}

void cleanupOldImportRuns(int keepCount)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir rootDir(base);
    if (!rootDir.exists()) return;

    const QStringList runDirs = rootDir.entryList(QStringList() << ui::config::kImportRunGlobPattern,
                                                  QDir::Dirs | QDir::NoDotAndDotDot,
                                                  QDir::Time);
    if (runDirs.size() <= keepCount) return;

    for (int i = keepCount; i < runDirs.size(); ++i) {
        QDir runDir(rootDir.filePath(runDirs[i]));
        runDir.removeRecursively();
    }
}

}