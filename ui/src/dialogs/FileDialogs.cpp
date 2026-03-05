#include "ui/dialogs/FileDialogs.h"

#include <QFileDialog>
#include <QObject>

namespace ui::dialogs {

QStringList pickImportFiles(QWidget* parent, const QString& filter)
{
    return QFileDialog::getOpenFileNames(parent, QObject::tr("Select PDF"), QString(), filter);
}

QString pickExportFile(QWidget* parent, const QString& filter)
{
    return QFileDialog::getSaveFileName(parent, QObject::tr("Export File"), QString(), filter);
}

QString pickNewStorageFile(QWidget* parent)
{
    return QFileDialog::getSaveFileName(parent, QObject::tr("New File"), QString(), QObject::tr("Database (*.db)"));
}

QString pickOpenStorageFile(QWidget* parent)
{
    return QFileDialog::getOpenFileName(parent, QObject::tr("Open File"), QString(), QObject::tr("Database (*.db)"));
}

QString pickSaveStorageFileAs(QWidget* parent)
{
    return QFileDialog::getSaveFileName(parent, QObject::tr("Save File As"), QString(), QObject::tr("Database (*.db)"));
}

}
