#include "ui/workflows/FileWorkflow.h"

#include <QWidget>

#include "ui/dialogs/FileDialogs.h"

namespace ui::workflows {

FileWorkflow::FileWorkflow(QWidget* parentWindow, QObject* parent)
    : QObject(parent)
    , parentWindow_(parentWindow)
{
}

void FileWorkflow::requestNewFile()
{
    const QString file = ui::dialogs::pickNewStorageFile(parentWindow_);
    if (file.isEmpty()) return;
    emit newFileRequested(file);
}

void FileWorkflow::requestOpenFile()
{
    const QString file = ui::dialogs::pickOpenStorageFile(parentWindow_);
    if (file.isEmpty()) return;
    emit openFileRequested(file);
}

void FileWorkflow::requestSaveFile()
{
    emit saveFileRequested();
}

void FileWorkflow::requestSaveFileAs()
{
    const QString file = ui::dialogs::pickSaveStorageFileAs(parentWindow_);
    if (file.isEmpty()) return;
    emit saveFileAsRequested(file);
}

}
