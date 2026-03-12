/**
 * @file ui/include/ui/dialogs/FileDialogs.h
 * @brief Declares shared native file dialog helpers used by the desktop UI.
 */

#pragma once

#include <QString>
#include <QStringList>

class QWidget;

namespace ui::dialogs {

/** @brief Opens a multi-select file dialog for statement import files. */
QStringList pickImportFiles(QWidget* parent, const QString& filter);
/** @brief Opens a save dialog for export targets. */
QString pickExportFile(QWidget* parent, const QString& filter);

/** @brief Opens a save dialog for creating a new storage database. */
QString pickNewStorageFile(QWidget* parent);
/** @brief Opens a file dialog for selecting an existing storage database. */
QString pickOpenStorageFile(QWidget* parent);
/** @brief Opens a save dialog for choosing a database target path. */
QString pickSaveStorageFileAs(QWidget* parent);

}
