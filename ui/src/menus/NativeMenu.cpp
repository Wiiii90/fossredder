#include "ui/menus/NativeMenu.h"
#include "ui/actions/UiActions.h"

#include <QQmlContext>
#include <QMenu>

NativeMenu::NativeMenu(QMainWindow* window, UiActions* actions, QQmlContext* qmlContext, QObject* parent)
    : QObject(parent)
    , m_window(window)
    , m_qmlContext(qmlContext)
    , m_menuBar(nullptr)
{
    setupMenus(actions);
}

void NativeMenu::setupMenus(UiActions* actions)
{
    if (!m_window || !actions) return;

    m_menuBar = m_window->menuBar();

    QMenu* fileMenu = m_menuBar->addMenu(tr("File"));
    fileMenu->addAction(actions->newFileAction());
    fileMenu->addAction(actions->openFileAction());
    fileMenu->addAction(actions->saveFileAction());
    fileMenu->addAction(actions->saveFileAsAction());
    fileMenu->addSeparator();
    fileMenu->addAction(actions->importAction());
    fileMenu->addAction(actions->exportAction());
    fileMenu->addSeparator();
    fileMenu->addAction(actions->quitAction());

    QMenu* viewMenu = m_menuBar->addMenu(tr("View"));
    viewMenu->addAction(actions->toggleSidebarAction());

    QMenu* helpMenu = m_menuBar->addMenu(tr("Help"));
    helpMenu->addAction(actions->aboutAction());

    if (m_qmlContext) {
        m_qmlContext->setContextProperty("useNativeMenu", true);

        m_qmlContext->setContextProperty("actionNewFile", actions->newFileAction());
        m_qmlContext->setContextProperty("actionOpenFile", actions->openFileAction());
        m_qmlContext->setContextProperty("actionSaveFile", actions->saveFileAction());
        m_qmlContext->setContextProperty("actionSaveFileAs", actions->saveFileAsAction());

        m_qmlContext->setContextProperty("actionImport", actions->importAction());
        m_qmlContext->setContextProperty("actionExport", actions->exportAction());
        m_qmlContext->setContextProperty("actionQuit", actions->quitAction());
        m_qmlContext->setContextProperty("actionToggleSidebar", actions->toggleSidebarAction());
        m_qmlContext->setContextProperty("actionAbout", actions->aboutAction());
    }
}
