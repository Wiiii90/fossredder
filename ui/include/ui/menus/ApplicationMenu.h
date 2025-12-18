#pragma once

#include <QObject>
#include <QMenuBar>
#include <QMainWindow>

class QQmlContext;
class MenuActions;

class ApplicationMenu : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationMenu(QMainWindow* window, MenuActions* actions, QQmlContext* qmlContext, QObject* parent = nullptr);

private:
    void setupMenus(MenuActions* actions);

    QMainWindow* m_window;
    QQmlContext* m_qmlContext;

    QMenuBar* m_menuBar;
};
