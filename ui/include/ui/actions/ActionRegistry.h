#pragma once

#include <QObject>

class QAction;

namespace ui {

class ActionRegistry : public QObject {
    Q_OBJECT
public:
    explicit ActionRegistry(QObject* parent = nullptr);

    QAction* importAction() const;
    QAction* exportAction() const;
    QAction* annualAction() const;
    QAction* actorsAction() const;
    QAction* propertiesAction() const;

private:
    QAction* m_import = nullptr;
    QAction* m_export = nullptr;
    QAction* m_annual = nullptr;
    QAction* m_actors = nullptr;
    QAction* m_properties = nullptr;
};

}