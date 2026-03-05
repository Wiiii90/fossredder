#pragma once

#include <QString>
#include <QStringList>

class QWidget;

namespace ui::dialogs {

QStringList pickImportFiles(QWidget* parent, const QString& filter);
QString pickExportFile(QWidget* parent, const QString& filter);

QString pickNewStorageFile(QWidget* parent);
QString pickOpenStorageFile(QWidget* parent);
QString pickSaveStorageFileAs(QWidget* parent);

}
