#include "ui/dialogs/FileDialogs.h"

#include <QFileDialog>
#include <QObject>

#include "ui/text/Text.h"

namespace ui::dialogs {

QStringList pickImportFiles(QWidget* parent, const QString& filter)
{
    return QFileDialog::getOpenFileNames(parent, QObject::tr(ui::text::dialogs::kSelectPdfTitle), QString(), filter);
}

QString pickExportFile(QWidget* parent, const QString& filter)
{
    return QFileDialog::getSaveFileName(parent, QObject::tr(ui::text::dialogs::kExportFileTitle), QString(), filter);
}

QString pickNewStorageFile(QWidget* parent)
{
    return QFileDialog::getSaveFileName(parent,
                                        QObject::tr(ui::text::dialogs::kNewFileTitle),
                                        QString(),
                                        QObject::tr(ui::text::dialogs::kDatabaseFilter));
}

QString pickOpenStorageFile(QWidget* parent)
{
    return QFileDialog::getOpenFileName(parent,
                                        QObject::tr(ui::text::dialogs::kOpenFileTitle),
                                        QString(),
                                        QObject::tr(ui::text::dialogs::kDatabaseFilter));
}

QString pickSaveStorageFileAs(QWidget* parent)
{
    return QFileDialog::getSaveFileName(parent,
                                        QObject::tr(ui::text::dialogs::kSaveFileAsTitle),
                                        QString(),
                                        QObject::tr(ui::text::dialogs::kDatabaseFilter));
}

}
