#include "ui/UiActions.h"
#include "MainWindow.h"

UiActions::UiActions(MainWindow* window)
    : QObject(window), m_window(window) {
}

void UiActions::importFile() {
    if (!m_window) return;
    QMetaObject::invokeMethod(m_window, "onImport", Qt::QueuedConnection);
}

void UiActions::newFile() {
    if (!m_window) return;
    QMetaObject::invokeMethod(m_window, "onNewFile", Qt::QueuedConnection);
}

void UiActions::openFile() {
    if (!m_window) return;
    QMetaObject::invokeMethod(m_window, "onOpenFile", Qt::QueuedConnection);
}

void UiActions::saveFile() {
    if (!m_window) return;
    QMetaObject::invokeMethod(m_window, "onSaveFile", Qt::QueuedConnection);
}

void UiActions::saveFileAs() {
    if (!m_window) return;
    QMetaObject::invokeMethod(m_window, "onSaveFileAs", Qt::QueuedConnection);
}
