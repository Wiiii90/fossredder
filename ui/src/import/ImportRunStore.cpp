#include "ui/import/ImportRunStore.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

namespace ui::importing {

ImportRunInfo createImportRunInfo()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir rootDir(base);
    rootDir.mkpath(".");

    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");

    int suffix = 1;
    while (true) {
        const QString runName = QStringLiteral("%1_import_%2").arg(timestamp).arg(suffix);
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

    const QStringList runDirs = rootDir.entryList(QStringList() << "*_import_*",
                                                  QDir::Dirs | QDir::NoDotAndDotDot,
                                                  QDir::Time);
    if (runDirs.size() <= keepCount) return;

    for (int i = keepCount; i < runDirs.size(); ++i) {
        QDir runDir(rootDir.filePath(runDirs[i]));
        runDir.removeRecursively();
    }
}

}