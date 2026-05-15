/**
 * @file ui/src/window/MainWindowContext.cpp
 * @brief Implements main window service wiring between widgets, QML and UI services.
 */

#include "ui/shell/window/MainWindowContext.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QQmlContext>
#include <QVariant>
#include <QWidget>

#include "MainWindow.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/QmlContracts.h"
#include "ui/platform/filesystem/FileSystemBrowser.h"
#include "ui/platform/localization/LanguageService.h"
#include "ui/platform/dialogs/FileDialogs.h"
#include "ui/shared/observability/Origins.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/state/status/StatusState.h"
#include "ui/workspace/WorkspaceFacade.h"
#include "ui/shared/text/Text.h"
#include "ui/shell/window/MainWindowTrace.h"

namespace ui::window {

MainWindowServices installMainWindowContext(QQmlContext &qmlContext,
                                            QWidget *parentWindow,
                                            QObject *parent) {
  MainWindowServices services;
  auto *mainWindow = qobject_cast<MainWindow *>(parentWindow);
  services.actions = new ui::Actions(parent);
  services.navigation = new ui::NavigationState(parent);
  services.workspace = new ui::WorkspaceFacade(parent);
  services.fileSystemBrowser = new ui::FileSystemBrowser(parent);
  auto *application = qobject_cast<QApplication *>(QCoreApplication::instance());
  services.languageService = new ui::LanguageService(application, qmlContext.engine(), parent);
  services.settingsViewModel = new ui::SettingsViewModel(parent);
  services.status = new ui::StatusState(parent);
  services.status->setText(ui::text::status::ready());

    if (mainWindow) {
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kActions,
                                      services.actions);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kNavigation,
                                      services.navigation);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kSession,
                                      services.workspace);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kWorkspace,
                                      services.workspace);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kWorkspaceFacade,
                                      services.workspace);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kFileSystemBrowser,
                                      services.fileSystemBrowser);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kLanguageService,
                                      services.languageService);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kSettingsViewModel,
                                      services.settingsViewModel);
    mainWindow->setQmlContextProperty(ui::qml::contracts::context::kStatus,
                                      services.status);
  } else {
    qmlContext.setContextProperty(ui::qml::contracts::context::kActions,
                                  services.actions);
    qmlContext.setContextProperty(ui::qml::contracts::context::kNavigation,
                                  services.navigation);
    qmlContext.setContextProperty(ui::qml::contracts::context::kSession,
                                  services.workspace);
    qmlContext.setContextProperty(ui::qml::contracts::context::kWorkspace,
                                  services.workspace);
    qmlContext.setContextProperty(ui::qml::contracts::context::kWorkspaceFacade,
                                  services.workspace);
    qmlContext.setContextProperty(ui::qml::contracts::context::kFileSystemBrowser,
                                  services.fileSystemBrowser);
    qmlContext.setContextProperty(ui::qml::contracts::context::kLanguageService,
                                  services.languageService);
    qmlContext.setContextProperty(ui::qml::contracts::context::kSettingsViewModel,
                                  services.settingsViewModel);
    qmlContext.setContextProperty(ui::qml::contracts::context::kStatus,
                                  services.status);
  }

#ifdef QT_DEBUG
  if (mainWindow)
    mainWindow->setQmlContextValue(ui::qml::contracts::context::kIsDebugBuild,
                                   QVariant(true));
  else
    qmlContext.setContextProperty(ui::qml::contracts::context::kIsDebugBuild,
                                  QVariant(true));
#else
  if (mainWindow)
    mainWindow->setQmlContextValue(ui::qml::contracts::context::kIsDebugBuild,
                                   QVariant(false));
  else
    qmlContext.setContextProperty(ui::qml::contracts::context::kIsDebugBuild,
                                  QVariant(false));
#endif

  return services;
}

void wireMainWindowActions(MainWindow &window,
                           const MainWindowServices &services,
                           const std::function<void()> &showAbout) {
  if (!services.actions)
    return;

  auto *actions = services.actions;
  QObject::connect(actions->newFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickNewStorageFile(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested new file", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.newFileRequested(file);
                   });
  QObject::connect(actions->openFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickOpenStorageFile(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested open file", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.openFileRequested(file);
                   });
  QObject::connect(actions->saveFileAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested save file");
                     emit window.saveFileRequested();
                   });
  QObject::connect(actions->saveFileAsAction(), &QAction::triggered, &window,
                   [&window](bool) {
                     const QString file = ui::dialogs::pickSaveStorageFileAs(&window);
                     if (file.isEmpty()) return;
                     ui::window::reportMainWindowFlow(
                         ui::observability::origins::mainWindow::kActionRouting,
                         "UI requested save file as", core::errors::ErrorSeverity::Info,
                         ui::window::makePathContext(file));
                     emit window.saveFileAsRequested(file);
                   });

  QObject::connect(actions->quitAction(), &QAction::triggered, &window,
                   &QWidget::close);
  QObject::connect(actions->aboutAction(), &QAction::triggered, &window,
                   [showAbout]() {
                     if (showAbout)
                       showAbout();
                   });

  QObject::connect(
      actions, &ui::Actions::importBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &filter) {
        const QStringList files = ui::dialogs::pickImportFiles(&window, filter);
        if (!files.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected import files", core::errors::ErrorSeverity::Info,
              ui::window::makeFileListContext(files));
          emit actions->importFilesSelected(files);
          if (files.size() == 1)
            emit actions->importFileSelected(files.first());
          if (status)
            status->setText(
                ui::text::mainWindow::selectedStatusPattern().arg(files.front()));
        }
      });

  QObject::connect(
      actions, &ui::Actions::exportBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &filter) {
        const QString file = ui::dialogs::pickExportFile(&window, filter);
        if (!file.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected export path", core::errors::ErrorSeverity::Info,
              ui::window::makePathContext(file));
          emit actions->exportFileSelected(file);
          if (status)
            status->setText(
                ui::text::mainWindow::exportPathStatusPattern().arg(file));
        }
      });

  QObject::connect(
      actions, &ui::Actions::exportDirectoryBrowseRequested, &window,
      [&window, actions, status = services.status](const QString &title) {
        const QString directory = ui::dialogs::pickExportDirectory(&window, title);
        if (!directory.isEmpty()) {
          ui::window::reportMainWindowFlow(
              ui::observability::origins::mainWindow::kActionRouting,
              "UI selected export directory", core::errors::ErrorSeverity::Info,
              ui::window::makePathContext(directory));
          emit actions->exportDirectorySelected(directory);
          if (status)
            status->setText(
                ui::text::mainWindow::exportPathStatusPattern().arg(directory));
        }
      });
}

} // namespace ui::window
