#include "ui/controllers/UiFileSystem.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

UiFileSystem::UiFileSystem(QObject* parent) : QObject(parent) {}

QVariantList UiFileSystem::listDir(const QString& path) const
{
    QVariantList out;
    QDir d(path);
    if (!d.exists()) return out;
    // list directories first, then files
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

bool UiFileSystem::exists(const QString& path) const
{
    QFileInfo fi(path);
    return fi.exists();
}

QString UiFileSystem::homeDir() const
{
    return QDir::homePath();
}

QString UiFileSystem::appDir() const
{
    // Return actual application directory (where the exe lives)
    return QCoreApplication::applicationDirPath();
}
