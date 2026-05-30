#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "ui/shell/AppActions.h"
#include "ui/platform/filesystem/FileSystemBrowser.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "ui/workflows/annual/AnnualWorkflow.h"
#include "ui/workflows/export/ExportWorkflow.h"
#include "ui/platform/localization/LanguageService.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/state/navigation/NavigationState.h"
#include "ui/state/status/StatusState.h"
#include "ui/state/session/WorkspaceSessionState.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui::bootstrap {

class AppContext : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AppContext)
    QML_SINGLETON
    Q_PROPERTY(ui::Actions* actions READ actions WRITE setActions NOTIFY actionsChanged)
    Q_PROPERTY(ui::NavigationState* navigation READ navigation WRITE setNavigation NOTIFY navigationChanged)
    Q_PROPERTY(ui::WorkspaceFacade* session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(ui::SessionState* sessionState READ sessionState NOTIFY sessionStateChanged)
    Q_PROPERTY(ui::WorkspaceFacade* workspace READ workspace WRITE setWorkspace NOTIFY workspaceChanged)
    Q_PROPERTY(ui::WorkspaceFacade* workspaceFacade READ workspaceFacade WRITE setWorkspaceFacade NOTIFY workspaceFacadeChanged)
    Q_PROPERTY(ui::FileSystemBrowser* fileSystemBrowser READ fileSystemBrowser WRITE setFileSystemBrowser NOTIFY fileSystemBrowserChanged)
    Q_PROPERTY(ui::StatusState* status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(ui::AnalysisWorkflow* analysisWorkflow READ analysisWorkflow WRITE setAnalysisWorkflow NOTIFY analysisWorkflowChanged)
    Q_PROPERTY(ui::AnnualWorkflow* annualWorkflow READ annualWorkflow WRITE setAnnualWorkflow NOTIFY annualWorkflowChanged)
    Q_PROPERTY(ui::ExportWorkflow* exportWorkflow READ exportWorkflow WRITE setExportWorkflow NOTIFY exportWorkflowChanged)
    Q_PROPERTY(ui::ImportWorkflow* importWorkflow READ importWorkflow WRITE setImportWorkflow NOTIFY importWorkflowChanged)
    Q_PROPERTY(ui::LanguageService* languageService READ languageService WRITE setLanguageService NOTIFY languageServiceChanged)
    Q_PROPERTY(ui::SettingsViewModel* settingsViewModel READ settingsViewModel WRITE setSettingsViewModel NOTIFY settingsViewModelChanged)
    Q_PROPERTY(bool isDebugBuild READ isDebugBuild WRITE setIsDebugBuild NOTIFY isDebugBuildChanged)

public:
    explicit AppContext(QObject* parent = nullptr);

    ui::Actions* actions() const noexcept { return actions_; }
    ui::NavigationState* navigation() const noexcept { return navigation_; }
    ui::WorkspaceFacade* session() const noexcept { return session_; }
    ui::WorkspaceFacade* workspace() const noexcept { return session_; }
    ui::WorkspaceFacade* workspaceFacade() const noexcept { return session_; }
    ui::SessionState* sessionState() const noexcept { return sessionState_; }
    ui::FileSystemBrowser* fileSystemBrowser() const noexcept { return fileSystemBrowser_; }
    ui::StatusState* status() const noexcept { return status_; }
    ui::AnalysisWorkflow* analysisWorkflow() const noexcept { return analysisWorkflow_; }
    ui::AnnualWorkflow* annualWorkflow() const noexcept { return annualWorkflow_; }
    ui::ExportWorkflow* exportWorkflow() const noexcept { return exportWorkflow_; }
    ui::ImportWorkflow* importWorkflow() const noexcept { return importWorkflow_; }
    ui::LanguageService* languageService() const noexcept { return languageService_; }
    ui::SettingsViewModel* settingsViewModel() const noexcept { return settingsViewModel_; }
    bool isDebugBuild() const noexcept { return isDebugBuild_; }

    void setActions(ui::Actions* value);
    void setNavigation(ui::NavigationState* value);
    void setSession(ui::WorkspaceFacade* value);
    void setWorkspace(ui::WorkspaceFacade* value);
    void setWorkspaceFacade(ui::WorkspaceFacade* value);
    void setFileSystemBrowser(ui::FileSystemBrowser* value);
    void setStatus(ui::StatusState* value);
    void setAnalysisWorkflow(ui::AnalysisWorkflow* value);
    void setAnnualWorkflow(ui::AnnualWorkflow* value);
    void setExportWorkflow(ui::ExportWorkflow* value);
    void setImportWorkflow(ui::ImportWorkflow* value);
    void setLanguageService(ui::LanguageService* value);
    void setSettingsViewModel(ui::SettingsViewModel* value);
    void setIsDebugBuild(bool value);

signals:
    void actionsChanged();
    void navigationChanged();
    void sessionChanged();
    void sessionStateChanged();
    void workspaceChanged();
    void workspaceFacadeChanged();
    void fileSystemBrowserChanged();
    void statusChanged();
    void analysisWorkflowChanged();
    void annualWorkflowChanged();
    void exportWorkflowChanged();
    void importWorkflowChanged();
    void languageServiceChanged();
    void settingsViewModelChanged();
    void isDebugBuildChanged();

private:
    ui::Actions* actions_ = nullptr;
    ui::NavigationState* navigation_ = nullptr;
    ui::WorkspaceFacade* session_ = nullptr;
    ui::SessionState* sessionState_ = nullptr;
    ui::FileSystemBrowser* fileSystemBrowser_ = nullptr;
    ui::StatusState* status_ = nullptr;
    ui::AnalysisWorkflow* analysisWorkflow_ = nullptr;
    ui::AnnualWorkflow* annualWorkflow_ = nullptr;
    ui::ExportWorkflow* exportWorkflow_ = nullptr;
    ui::ImportWorkflow* importWorkflow_ = nullptr;
    ui::LanguageService* languageService_ = nullptr;
    ui::SettingsViewModel* settingsViewModel_ = nullptr;
    bool isDebugBuild_ = false;
};

} // namespace ui::bootstrap
