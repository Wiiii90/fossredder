/**
 * @file ui/include/ui/window/MainWindowContext.h
 * @brief Declarations for the UI MainWindowContext component.
 */

#pragma once

#include <functional>

class MainWindow;
class QObject;
class QQmlContext;
class QWidget;

namespace ui {
class Actions;
class StateFacade;
class StatusState;
}

namespace ui::window {

struct MainWindowServices {
    ui::Actions* actions = nullptr;
    ui::StateFacade* dataSession = nullptr;
    ui::StatusState* status = nullptr;
};

MainWindowServices installMainWindowContext(QQmlContext& qmlContext, QWidget* parentWindow, QObject* parent);
void wireMainWindowActions(MainWindow& window,
                           const MainWindowServices& services,
                           const std::function<void()>& showAbout);

}
