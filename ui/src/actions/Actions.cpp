#include "ui/actions/Actions.h"

#include <QAction>

namespace ui {

Actions::Actions(QObject* parent)
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

QAction* Actions::newFileAction() const { return newFileAction_; }
QAction* Actions::openFileAction() const { return openFileAction_; }
QAction* Actions::saveFileAction() const { return saveFileAction_; }
QAction* Actions::saveFileAsAction() const { return saveFileAsAction_; }
QAction* Actions::importAction() const { return importAction_; }
QAction* Actions::exportAction() const { return exportAction_; }
QAction* Actions::quitAction() const { return quitAction_; }
QAction* Actions::toggleSidebarAction() const { return toggleSidebarAction_; }
QAction* Actions::aboutAction() const { return aboutAction_; }

void Actions::newFile() { if (newFileAction_) newFileAction_->trigger(); }
void Actions::openFile() { if (openFileAction_) openFileAction_->trigger(); }
void Actions::saveFile() { if (saveFileAction_) saveFileAction_->trigger(); }
void Actions::saveFileAs() { if (saveFileAsAction_) saveFileAsAction_->trigger(); }
void Actions::importFile() { if (importAction_) importAction_->trigger(); }
void Actions::exportFile() { if (exportAction_) exportAction_->trigger(); }

void Actions::browseImportPdf()
{
    emit importBrowseRequested(tr("PDF Files (*.pdf)"));
}

void Actions::browseExportFile()
{
    emit exportBrowseRequested(tr("Excel Files (*.xlsx);;CSV Files (*.csv)"));
}

}
