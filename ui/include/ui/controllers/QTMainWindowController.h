#pragma once

#include <QObject>
#include <QList>

namespace ui {

class QTMainController;
class ActionRegistry;
class BaseView;
class MainWindow;

class QTMainWindowController : public QObject {
    Q_OBJECT
public:
    explicit QTMainWindowController(MainWindow* mainWindow, QObject* parent = nullptr);

    QTMainController* qtController() const { return m_qt_controller; }
    ActionRegistry* actionRegistry() const { return m_action_registry; }
    const QList<BaseView*>& views() const { return m_views; }

    // Setup UI related elements
    void setupMenus(MainWindow* mainWindow);
    void setupToolbar(MainWindow* mainWindow);

private:
    MainWindow* m_mainWindow = nullptr;
    QTMainController* m_qt_controller = nullptr;
    ActionRegistry* m_action_registry = nullptr;
    QList<BaseView*> m_views;
};

}
