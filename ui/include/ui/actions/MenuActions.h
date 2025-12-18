#pragma once

#include <QObject>
#include <QAction>

class MenuActions : public QObject
{
    Q_OBJECT
public:
    explicit MenuActions(QObject* parent = nullptr);

    QAction* importAction() const;
    QAction* exportAction() const;
    QAction* quitAction() const;
    QAction* toggleSidebarAction() const;
    QAction* aboutAction() const;

private:
    QAction* m_importAction;
    QAction* m_exportAction;
    QAction* m_quitAction;
    QAction* m_toggleSidebarAction;
    QAction* m_aboutAction;
};
