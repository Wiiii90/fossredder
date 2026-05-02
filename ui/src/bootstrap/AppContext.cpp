#include "ui/bootstrap/AppContext.h"

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

void AppContext::setSession(ui::StateFacade* value)
{
    assignIfChanged(session_, value, this, &AppContext::sessionChanged);
}

void AppContext::setFileSystemController(ui::FileSystemController* value)
{
    assignIfChanged(fileSystemController_, value, this, &AppContext::fileSystemControllerChanged);
}

void AppContext::setStatus(ui::StatusState* value)
{
    assignIfChanged(status_, value, this, &AppContext::statusChanged);
}

void AppContext::setStorageController(ui::StorageController* value)
{
    assignIfChanged(storageController_, value, this, &AppContext::storageControllerChanged);
}

void AppContext::setAnnualController(ui::AnnualController* value)
{
    assignIfChanged(annualController_, value, this, &AppContext::annualControllerChanged);
}

void AppContext::setActorController(ui::ActorController* value)
{
    assignIfChanged(actorController_, value, this, &AppContext::actorControllerChanged);
}

void AppContext::setPropertyController(ui::PropertyController* value)
{
    assignIfChanged(propertyController_, value, this, &AppContext::propertyControllerChanged);
}

void AppContext::setContractController(ui::ContractController* value)
{
    assignIfChanged(contractController_, value, this, &AppContext::contractControllerChanged);
}

void AppContext::setStatementController(ui::StatementController* value)
{
    assignIfChanged(statementController_, value, this, &AppContext::statementControllerChanged);
}

void AppContext::setTransactionController(ui::TransactionController* value)
{
    assignIfChanged(transactionController_, value, this, &AppContext::transactionControllerChanged);
}

void AppContext::setDraftController(ui::DraftController* value)
{
    assignIfChanged(draftController_, value, this, &AppContext::draftControllerChanged);
}

void AppContext::setAnalysisController(ui::AnalysisController* value)
{
    assignIfChanged(analysisController_, value, this, &AppContext::analysisControllerChanged);
}

void AppContext::setExportController(ui::ExportController* value)
{
    assignIfChanged(exportController_, value, this, &AppContext::exportControllerChanged);
}

void AppContext::setImportController(ui::ImportController* value)
{
    assignIfChanged(importController_, value, this, &AppContext::importControllerChanged);
}

void AppContext::setLanguageController(ui::LanguageController* value)
{
    assignIfChanged(languageController_, value, this, &AppContext::languageControllerChanged);
}

void AppContext::setSettingsController(ui::SettingsController* value)
{
    assignIfChanged(settingsController_, value, this, &AppContext::settingsControllerChanged);
}

void AppContext::setIsDebugBuild(bool value)
{
    if (isDebugBuild_ == value) return;
    isDebugBuild_ = value;
    emit isDebugBuildChanged();
}

} // namespace ui::bootstrap
