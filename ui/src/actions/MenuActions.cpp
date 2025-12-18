#include "ui/actions/MenuActions.h"

MenuActions::MenuActions(QObject* parent)
    : QObject(parent)
    , m_importAction(new QAction(tr("Import..."), this))
    , m_exportAction(new QAction(tr("Export..."), this))
    , m_quitAction(new QAction(tr("Quit"), this))
    , m_toggleSidebarAction(new QAction(tr("Toggle Sidebar"), this))
    , m_aboutAction(new QAction(tr("About"), this))
{
    m_importAction->setObjectName("actionImport");
    m_exportAction->setObjectName("actionExport");
    m_quitAction->setObjectName("actionQuit");
    m_toggleSidebarAction->setObjectName("actionToggleSidebar");
    m_aboutAction->setObjectName("actionAbout");
}

QAction* MenuActions::importAction() const { return m_importAction; }
QAction* MenuActions::exportAction() const { return m_exportAction; }
QAction* MenuActions::quitAction() const { return m_quitAction; }
QAction* MenuActions::toggleSidebarAction() const { return m_toggleSidebarAction; }
QAction* MenuActions::aboutAction() const { return m_aboutAction; }
