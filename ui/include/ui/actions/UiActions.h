#pragma once

#include <QObject>
#include <QStringList>

class QAction;

class UiActions : public QObject {
    Q_OBJECT
    Q_PROPERTY(QAction* newFileAction READ newFileAction CONSTANT)
    Q_PROPERTY(QAction* openFileAction READ openFileAction CONSTANT)
    Q_PROPERTY(QAction* saveFileAction READ saveFileAction CONSTANT)
    Q_PROPERTY(QAction* saveFileAsAction READ saveFileAsAction CONSTANT)
    Q_PROPERTY(QAction* importAction READ importAction CONSTANT)
    Q_PROPERTY(QAction* exportAction READ exportAction CONSTANT)
    Q_PROPERTY(QAction* quitAction READ quitAction CONSTANT)
    Q_PROPERTY(QAction* toggleSidebarAction READ toggleSidebarAction CONSTANT)
    Q_PROPERTY(QAction* aboutAction READ aboutAction CONSTANT)

public:
    explicit UiActions(QObject* parent = nullptr);

    QAction* newFileAction() const;
    QAction* openFileAction() const;
    QAction* saveFileAction() const;
    QAction* saveFileAsAction() const;
    QAction* importAction() const;
    QAction* exportAction() const;
    QAction* quitAction() const;
    QAction* toggleSidebarAction() const;
    QAction* aboutAction() const;

    Q_INVOKABLE void newFile();
    Q_INVOKABLE void openFile();
    Q_INVOKABLE void saveFile();
    Q_INVOKABLE void saveFileAs();
    Q_INVOKABLE void importFile();
    Q_INVOKABLE void exportFile();

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
    QAction* importAction_;
    QAction* exportAction_;
    QAction* quitAction_;
    QAction* toggleSidebarAction_;
    QAction* aboutAction_;
};
