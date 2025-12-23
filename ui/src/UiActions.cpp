#include "ui/UiActions.h"
#include "MainWindow.h"

UiActions::UiActions(MainWindow* window)
    : QObject(window), m_window(window)
{
}

void UiActions::importFile()
{
    if (!m_window)
        return;

    QMetaObject::invokeMethod(m_window, "onImport", Qt::QueuedConnection);
}
