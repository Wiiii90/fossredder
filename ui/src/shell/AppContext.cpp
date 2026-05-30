/**
 * @file ui/src/bootstrap/AppContext.cpp
 * @brief Implements the QML application context for the UI boundary.
 */

#include "ui/shell/AppContext.h"

namespace ui::bootstrap {

AppContext::AppContext(QObject* parent)
    : QObject(parent)
{
}

namespace {

template <typename TSignal, typename TValue>
void assignIfChanged(TValue*& target, TValue* value, AppContext* self, TSignal signal)
{
    if (target == value) return;
    target = value;
    emit (self->*signal)();
}

} // namespace

void AppContext::setActions(ui::Actions* value)
{
    assignIfChanged(actions_, value, this, &AppContext::actionsChanged);
}

void AppContext::setNavigation(ui::NavigationState* value)
{
    assignIfChanged(navigation_, value, this, &AppContext::navigationChanged);
}

void AppContext::setSession(ui::WorkspaceFacade* value)
{
    if (session_ == value) return;
    session_ = value;
    sessionState_ = session_ ? session_->session() : nullptr;
    emit sessionChanged();
    emit sessionStateChanged();
    emit workspaceChanged();
    emit workspaceFacadeChanged();
}

void AppContext::setWorkspace(ui::WorkspaceFacade* value)
{
    setSession(value);
}

void AppContext::setWorkspaceFacade(ui::WorkspaceFacade* value)
{
    setSession(value);
}

void AppContext::setFileSystemBrowser(ui::FileSystemBrowser* value)
{
    assignIfChanged(fileSystemBrowser_, value, this, &AppContext::fileSystemBrowserChanged);
}

void AppContext::setStatus(ui::StatusState* value)
{
    assignIfChanged(status_, value, this, &AppContext::statusChanged);
}

void AppContext::setAnalysisWorkflow(ui::AnalysisWorkflow* value)
{
    assignIfChanged(analysisWorkflow_, value, this, &AppContext::analysisWorkflowChanged);
}

void AppContext::setAnnualWorkflow(ui::AnnualWorkflow* value)
{
    assignIfChanged(annualWorkflow_, value, this, &AppContext::annualWorkflowChanged);
}

void AppContext::setExportWorkflow(ui::ExportWorkflow* value)
{
    assignIfChanged(exportWorkflow_, value, this, &AppContext::exportWorkflowChanged);
}

void AppContext::setImportWorkflow(ui::ImportWorkflow* value)
{
    assignIfChanged(importWorkflow_, value, this, &AppContext::importWorkflowChanged);
}

void AppContext::setLanguageService(ui::LanguageService* value)
{
    assignIfChanged(languageService_, value, this, &AppContext::languageServiceChanged);
}

void AppContext::setSettingsViewModel(ui::SettingsViewModel* value)
{
    assignIfChanged(settingsViewModel_, value, this, &AppContext::settingsViewModelChanged);
}

void AppContext::setIsDebugBuild(bool value)
{
    if (isDebugBuild_ == value) return;
    isDebugBuild_ = value;
    emit isDebugBuildChanged();
}

} // namespace ui::bootstrap
