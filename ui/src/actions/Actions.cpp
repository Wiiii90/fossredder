#include "ui/actions/Actions.h"

#include <QAction>

namespace ui {

Actions::Actions(QObject* parent)
    : QObject(parent)
    , newFileAction_(new QAction(tr("New..."), this))
    , openFileAction_(new QAction(tr("Open..."), this))
    , saveFileAction_(new QAction(tr("Save"), this))
    , saveFileAsAction_(new QAction(tr("Save As..."), this))
    , quitAction_(new QAction(tr("Quit"), this))
    , aboutAction_(new QAction(tr("About"), this))
{
    newFileAction_->setObjectName("actionNewFile");
    openFileAction_->setObjectName("actionOpenFile");
    saveFileAction_->setObjectName("actionSaveFile");
    saveFileAsAction_->setObjectName("actionSaveFileAs");

    quitAction_->setObjectName("actionQuit");
    aboutAction_->setObjectName("actionAbout");
}

QAction* Actions::newFileAction() const { return newFileAction_; }
QAction* Actions::openFileAction() const { return openFileAction_; }
QAction* Actions::saveFileAction() const { return saveFileAction_; }
QAction* Actions::saveFileAsAction() const { return saveFileAsAction_; }
QAction* Actions::quitAction() const { return quitAction_; }
QAction* Actions::aboutAction() const { return aboutAction_; }

void Actions::newFile() { if (newFileAction_) newFileAction_->trigger(); }
void Actions::openFile() { if (openFileAction_) openFileAction_->trigger(); }
void Actions::saveFile() { if (saveFileAction_) saveFileAction_->trigger(); }
void Actions::saveFileAs() { if (saveFileAsAction_) saveFileAsAction_->trigger(); }

void Actions::browseImportPdf()
{
    emit importBrowseRequested(tr("PDF Files (*.pdf)"));
}

void Actions::browseExportFile()
{
    emit exportBrowseRequested(tr("Excel Files (*.xlsx);;CSV Files (*.csv)"));
}

}
