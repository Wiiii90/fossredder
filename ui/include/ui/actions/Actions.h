/**
 * @file ui/include/ui/actions/Actions.h
 * @brief Declares QAction wrappers exposed to the UI layer and QML.
 */

#pragma once

#include <QObject>
#include <QStringList>

class QAction;

namespace ui {

/**
 * @brief Owns the main window actions and exposes them to QML.
 */
class Actions : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAction* newFileAction READ newFileAction CONSTANT)
    Q_PROPERTY(QAction* openFileAction READ openFileAction CONSTANT)
    Q_PROPERTY(QAction* saveFileAction READ saveFileAction CONSTANT)
    Q_PROPERTY(QAction* saveFileAsAction READ saveFileAsAction CONSTANT)
    Q_PROPERTY(QAction* quitAction READ quitAction CONSTANT)
    Q_PROPERTY(QAction* aboutAction READ aboutAction CONSTANT)

public:
    explicit Actions(QObject* parent = nullptr);

    QAction* newFileAction() const;
    QAction* openFileAction() const;
    QAction* saveFileAction() const;
    QAction* saveFileAsAction() const;
    QAction* quitAction() const;
    QAction* aboutAction() const;

    Q_INVOKABLE void newFile();
    Q_INVOKABLE void openFile();
    Q_INVOKABLE void saveFile();
    Q_INVOKABLE void saveFileAs();

    Q_INVOKABLE void browseImportPdf();
    Q_INVOKABLE void browseExportFile();

signals:
    void importBrowseRequested(const QString& filter);
    void importFileSelected(const QString& path);
    void importFilesSelected(const QStringList& paths);
    void importFileDropped(const QString& path);
    void importFilesDropped(const QStringList& paths);
    void exportBrowseRequested(const QString& filter);
    void exportFileSelected(const QString& path);

private:
    QAction* newFileAction_;
    QAction* openFileAction_;
    QAction* saveFileAction_;
    QAction* saveFileAsAction_;
    QAction* quitAction_;
    QAction* aboutAction_;
};

}
