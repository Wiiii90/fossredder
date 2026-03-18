/**
 * @file ui/src/controllers/FileSystemController.cpp
 * @brief Implements the UI filesystem helper exposed to QML.
 */

#include "ui/controllers/FileSystemController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include "ui/payload/PayloadKeys.h"

namespace ui {

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
        m[payload::keys::common::kName] = fi.fileName();
        m[payload::keys::fileSystem::kPath] = fi.absoluteFilePath();
        m[payload::keys::fileSystem::kIsDir] = fi.isDir();
        out.append(m);
    }
    return out;
}

QString FileSystemController::appDir() const
{
    return QCoreApplication::applicationDirPath();
}

}
