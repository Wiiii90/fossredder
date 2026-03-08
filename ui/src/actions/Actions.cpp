#include "ui/actions/Actions.h"

#include <QAction>

#include "ui/config/Defaults.h"
#include "ui/text/Text.h"

namespace ui {

Actions::Actions(QObject* parent)
    : QObject(parent)
    , newFileAction_(new QAction(tr(ui::text::actions::kNewFile), this))
    , openFileAction_(new QAction(tr(ui::text::actions::kOpenFile), this))
    , saveFileAction_(new QAction(tr(ui::text::actions::kSaveFile), this))
    , saveFileAsAction_(new QAction(tr(ui::text::actions::kSaveFileAs), this))
    , quitAction_(new QAction(tr(ui::text::actions::kQuit), this))
    , aboutAction_(new QAction(tr(ui::text::actions::kAbout), this))
{
    newFileAction_->setObjectName(ui::config::objectNames::kActionNewFile);
    openFileAction_->setObjectName(ui::config::objectNames::kActionOpenFile);
    saveFileAction_->setObjectName(ui::config::objectNames::kActionSaveFile);
    saveFileAsAction_->setObjectName(ui::config::objectNames::kActionSaveFileAs);

    quitAction_->setObjectName(ui::config::objectNames::kActionQuit);
    aboutAction_->setObjectName(ui::config::objectNames::kActionAbout);
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
    emit importBrowseRequested(tr(ui::text::dialogs::kImportPdfFilter));
}

void Actions::browseExportFile()
{
    emit exportBrowseRequested(tr(ui::text::dialogs::kExportFileFilter));
}

}
