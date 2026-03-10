#pragma once

#include <functional>

class MainWindow;
class QObject;
class QQuickWidget;
class QWidget;

namespace ui {
class Actions;
class StateFacade;
class StatusState;
}

namespace ui::workflows {
class FileWorkflow;
}

namespace ui::window {

struct MainWindowServices {
    ui::Actions* actions = nullptr;
    ui::StateFacade* dataSession = nullptr;
    ui::workflows::FileWorkflow* fileWorkflow = nullptr;
    ui::StatusState* status = nullptr;
};

MainWindowServices installMainWindowContext(QQuickWidget& quickWidget, QWidget* parentWindow, QObject* parent);
void wireMainWindowActions(MainWindow& window,
                           const MainWindowServices& services,
                           const std::function<void()>& showAbout);

}
