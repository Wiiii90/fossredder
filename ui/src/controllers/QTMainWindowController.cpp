#include "ui/controllers/QTMainWindowController.h"
#include "ui/controllers/QTMainController.h"
#include "ui/actions/ActionRegistry.h"
#include "ui/views/ViewFactory.h"
#include "ui/windows/MainWindow.h"
#include "ui/widgets/ToolBarWidget.h"
#include <QMenuBar>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QDebug>

namespace ui {

QTMainWindowController::QTMainWindowController(MainWindow* mainWindow, QObject* parent)
    : QObject(parent), m_mainWindow(mainWindow)
{
    m_qt_controller = new QTMainController(this);
    m_action_registry = new ActionRegistry(this);
    m_views = ViewFactory::createViews(mainWindow);
}

void QTMainWindowController::setupMenus(MainWindow* mainWindow) {
    if (!mainWindow) return;
    QMenu* fileMenu = mainWindow->menuBar()->addMenu(tr("File"));
    fileMenu->addAction(m_action_registry->importAction());
    fileMenu->addAction(m_action_registry->exportAction());
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), mainWindow, &MainWindow::close);

    QMenu* editMenu = mainWindow->menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Undo"));
    editMenu->addAction(tr("Redo"));
}

void QTMainWindowController::setupToolbar(MainWindow* mainWindow) {
    if (!mainWindow) return;
    QToolBar* tb = mainWindow->addToolBar(tr("Main"));
    tb->setObjectName("main_toolbar");
    tb->setMovable(false);
    tb->setFloatable(false);

    // Use existing ToolBarWidget
    // Note: ToolBarWidget expects an ActionRegistry pointer
    auto tbw = new ToolBarWidget(m_action_registry, mainWindow);
    tb->addWidget(tbw);
    tbw->registerViews(m_views);
}

}
