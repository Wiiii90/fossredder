#include "ui/actions/UiActions.h"

#include <QAction>

UiActions::UiActions(QObject* parent)
    : QObject(parent)
    , newFileAction_(new QAction(tr("New..."), this))
    , openFileAction_(new QAction(tr("Open..."), this))
    , saveFileAction_(new QAction(tr("Save"), this))
    , saveFileAsAction_(new QAction(tr("Save As..."), this))
    , importAction_(new QAction(tr("Import..."), this))
    , exportAction_(new QAction(tr("Export..."), this))
    , quitAction_(new QAction(tr("Quit"), this))
    , toggleSidebarAction_(new QAction(tr("Toggle Sidebar"), this))
    , aboutAction_(new QAction(tr("About"), this))
{
    newFileAction_->setObjectName("actionNewFile");
    openFileAction_->setObjectName("actionOpenFile");
    saveFileAction_->setObjectName("actionSaveFile");
    saveFileAsAction_->setObjectName("actionSaveFileAs");

    importAction_->setObjectName("actionImport");
    exportAction_->setObjectName("actionExport");
    quitAction_->setObjectName("actionQuit");
    toggleSidebarAction_->setObjectName("actionToggleSidebar");
    aboutAction_->setObjectName("actionAbout");
}

QAction* UiActions::newFileAction() const { return newFileAction_; }
QAction* UiActions::openFileAction() const { return openFileAction_; }
QAction* UiActions::saveFileAction() const { return saveFileAction_; }
QAction* UiActions::saveFileAsAction() const { return saveFileAsAction_; }
QAction* UiActions::importAction() const { return importAction_; }
QAction* UiActions::exportAction() const { return exportAction_; }
QAction* UiActions::quitAction() const { return quitAction_; }
QAction* UiActions::toggleSidebarAction() const { return toggleSidebarAction_; }
QAction* UiActions::aboutAction() const { return aboutAction_; }

void UiActions::newFile() { if (newFileAction_) newFileAction_->trigger(); }
void UiActions::openFile() { if (openFileAction_) openFileAction_->trigger(); }
void UiActions::saveFile() { if (saveFileAction_) saveFileAction_->trigger(); }
void UiActions::saveFileAs() { if (saveFileAsAction_) saveFileAsAction_->trigger(); }
void UiActions::importFile() { if (importAction_) importAction_->trigger(); }
void UiActions::exportFile() { if (exportAction_) exportAction_->trigger(); }

void UiActions::browseImportPdf()
{
    emit importBrowseRequested(tr("PDF Files (*.pdf)"));
}

void UiActions::browseExportFile()
{
    emit exportBrowseRequested(tr("Excel Files (*.xlsx);;CSV Files (*.csv)"));
}
