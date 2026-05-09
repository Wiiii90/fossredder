/**
 * @file ui/src/import/ImportRunStore.cpp
 * @brief Implementation of the UI ImportRunStore component.
 */

#include "ui/import/ImportRunStore.h"

#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#include "core/constants/import.h"

namespace {

/** @brief Converts an ASCII shared constant into a QString for Qt path helpers. */
QString fromCoreString(std::string_view value)
{
    return QString::fromLatin1(value.data());
}

/** @brief Builds the persisted directory name for a single import run. */
QString makeImportRunName(const QString& timestamp, int suffix)
{
    QString runName = QStringLiteral("%1_%2_%3");
    runName = runName.arg(timestamp);
    runName = runName.arg(fromCoreString(core::constants::importing::kRunNameImport));
    runName = runName.arg(suffix);
    return runName;
}

/** @brief Builds the glob used to enumerate persisted import run directories. */
QString importRunGlobPattern()
{
    return QStringLiteral("*_%1_*").arg(fromCoreString(core::constants::importing::kRunNameImport));
}

}

namespace ui::importing {

ImportRunInfo createImportRunInfo()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir rootDir(base);
    rootDir.mkpath(".");

    const QString timestamp = QDateTime::currentDateTimeUtc().toString(
        fromCoreString(core::constants::importing::runs::kTimestampFormat));

    int suffix = core::constants::importing::runs::kFirstSuffix;
    while (true) {
        const QString runName = makeImportRunName(timestamp, suffix);
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

    const QStringList runDirs = rootDir.entryList(QStringList() << importRunGlobPattern(),
                                                  QDir::Dirs | QDir::NoDotAndDotDot,
                                                  QDir::Time);
    if (runDirs.size() <= keepCount) return;

    for (int i = keepCount; i < runDirs.size(); ++i) {
        QDir runDir(rootDir.filePath(runDirs[i]));
        runDir.removeRecursively();
    }
}

}
