#include "ui/controllers/FileSystemController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

FileSystemController::FileSystemController(QObject* parent)
    : QObject(parent)
{
}

QVariantList FileSystemController::listDir(const QString& path) const
{
    QVariantList out;
    QDir d(path);
    if (!d.exists()) return out;
    QFileInfoList entries = d.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::DirsFirst | QDir::Name);
    for (const QFileInfo& fi : entries) {
        QVariantMap m;
        m["name"] = fi.fileName();
        m["path"] = fi.absoluteFilePath();
        m["isDir"] = fi.isDir();
        out.append(m);
    }
    return out;
}

bool FileSystemController::exists(const QString& path) const
{
    QFileInfo fi(path);
    return fi.exists();
}

QString FileSystemController::homeDir() const
{
    return QDir::homePath();
}

QString FileSystemController::appDir() const
{
    return QCoreApplication::applicationDirPath();
}
