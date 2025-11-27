#include "ui/actions/ActionRegistry.h"
#include <QAction>

namespace ui {

ActionRegistry::ActionRegistry(QObject* parent)
    : QObject(parent)
{
    m_import = new QAction(tr("Import..."), this);
    m_export = new QAction(tr("Export..."), this);
    m_annual = new QAction(tr("Annual"), this);
    m_actors = new QAction(tr("Actors"), this);
    m_properties = new QAction(tr("Properties"), this);
}

QAction* ActionRegistry::importAction() const { return m_import; }
QAction* ActionRegistry::exportAction() const { return m_export; }
QAction* ActionRegistry::annualAction() const { return m_annual; }
QAction* ActionRegistry::actorsAction() const { return m_actors; }
QAction* ActionRegistry::propertiesAction() const { return m_properties; }

}