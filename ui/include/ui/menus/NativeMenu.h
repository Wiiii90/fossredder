#pragma once

#include <QObject>
#include <QMenuBar>
#include <QMainWindow>

class QQmlContext;
class UiActions;

class NativeMenu : public QObject
{
    Q_OBJECT
public:
    explicit NativeMenu(QMainWindow* window, UiActions* actions, QQmlContext* qmlContext, QObject* parent = nullptr);

private:
    void setupMenus(UiActions* actions);

    QMainWindow* m_window;
    QQmlContext* m_qmlContext;

    QMenuBar* m_menuBar;
};
