#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "ui/actions/Actions.h"
#include "ui/controllers/ActorController.h"
#include "ui/controllers/AnalysisController.h"
#include "ui/controllers/AnnualController.h"
#include "ui/controllers/ContractController.h"
#include "ui/controllers/DraftController.h"
#include "ui/controllers/ExportController.h"
#include "ui/controllers/FileSystemController.h"
#include "ui/controllers/ImportController.h"
#include "ui/controllers/LanguageController.h"
#include "ui/controllers/PropertyController.h"
#include "ui/controllers/SettingsController.h"
#include "ui/controllers/StatementController.h"
#include "ui/controllers/StorageController.h"
#include "ui/controllers/TransactionController.h"
#include "ui/state/NavigationState.h"
#include "ui/state/StateFacade.h"
#include "ui/state/StatusState.h"

namespace ui::bootstrap {

class AppContext : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AppContext)
    QML_SINGLETON
    Q_PROPERTY(ui::Actions* actions READ actions WRITE setActions NOTIFY actionsChanged)
    Q_PROPERTY(ui::NavigationState* navigation READ navigation WRITE setNavigation NOTIFY navigationChanged)
    Q_PROPERTY(ui::StateFacade* session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(ui::FileSystemController* fileSystemController READ fileSystemController WRITE setFileSystemController NOTIFY fileSystemControllerChanged)
    Q_PROPERTY(ui::StatusState* status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(ui::StorageController* storageController READ storageController WRITE setStorageController NOTIFY storageControllerChanged)
    Q_PROPERTY(ui::AnnualController* annualController READ annualController WRITE setAnnualController NOTIFY annualControllerChanged)
    Q_PROPERTY(ui::ActorController* actorController READ actorController WRITE setActorController NOTIFY actorControllerChanged)
    Q_PROPERTY(ui::PropertyController* propertyController READ propertyController WRITE setPropertyController NOTIFY propertyControllerChanged)
    Q_PROPERTY(ui::ContractController* contractController READ contractController WRITE setContractController NOTIFY contractControllerChanged)
    Q_PROPERTY(ui::StatementController* statementController READ statementController WRITE setStatementController NOTIFY statementControllerChanged)
    Q_PROPERTY(ui::TransactionController* transactionController READ transactionController WRITE setTransactionController NOTIFY transactionControllerChanged)
    Q_PROPERTY(ui::DraftController* draftController READ draftController WRITE setDraftController NOTIFY draftControllerChanged)
    Q_PROPERTY(ui::AnalysisController* analysisController READ analysisController WRITE setAnalysisController NOTIFY analysisControllerChanged)
    Q_PROPERTY(ui::ExportController* exportController READ exportController WRITE setExportController NOTIFY exportControllerChanged)
    Q_PROPERTY(ui::ImportController* importController READ importController WRITE setImportController NOTIFY importControllerChanged)
    Q_PROPERTY(ui::LanguageController* languageController READ languageController WRITE setLanguageController NOTIFY languageControllerChanged)
    Q_PROPERTY(ui::SettingsController* settingsController READ settingsController WRITE setSettingsController NOTIFY settingsControllerChanged)
    Q_PROPERTY(bool isDebugBuild READ isDebugBuild WRITE setIsDebugBuild NOTIFY isDebugBuildChanged)

public:
    explicit AppContext(QObject* parent = nullptr);

    ui::Actions* actions() const noexcept { return actions_; }
    ui::NavigationState* navigation() const noexcept { return navigation_; }
    ui::StateFacade* session() const noexcept { return session_; }
    ui::FileSystemController* fileSystemController() const noexcept { return fileSystemController_; }
    ui::StatusState* status() const noexcept { return status_; }
    ui::StorageController* storageController() const noexcept { return storageController_; }
    ui::AnnualController* annualController() const noexcept { return annualController_; }
    ui::ActorController* actorController() const noexcept { return actorController_; }
    ui::PropertyController* propertyController() const noexcept { return propertyController_; }
    ui::ContractController* contractController() const noexcept { return contractController_; }
    ui::StatementController* statementController() const noexcept { return statementController_; }
    ui::TransactionController* transactionController() const noexcept { return transactionController_; }
    ui::DraftController* draftController() const noexcept { return draftController_; }
    ui::AnalysisController* analysisController() const noexcept { return analysisController_; }
    ui::ExportController* exportController() const noexcept { return exportController_; }
    ui::ImportController* importController() const noexcept { return importController_; }
    ui::LanguageController* languageController() const noexcept { return languageController_; }
    ui::SettingsController* settingsController() const noexcept { return settingsController_; }
    bool isDebugBuild() const noexcept { return isDebugBuild_; }

    void setActions(ui::Actions* value);
    void setNavigation(ui::NavigationState* value);
    void setSession(ui::StateFacade* value);
    void setFileSystemController(ui::FileSystemController* value);
    void setStatus(ui::StatusState* value);
    void setStorageController(ui::StorageController* value);
    void setAnnualController(ui::AnnualController* value);
    void setActorController(ui::ActorController* value);
    void setPropertyController(ui::PropertyController* value);
    void setContractController(ui::ContractController* value);
    void setStatementController(ui::StatementController* value);
    void setTransactionController(ui::TransactionController* value);
    void setDraftController(ui::DraftController* value);
    void setAnalysisController(ui::AnalysisController* value);
    void setExportController(ui::ExportController* value);
    void setImportController(ui::ImportController* value);
    void setLanguageController(ui::LanguageController* value);
    void setSettingsController(ui::SettingsController* value);
    void setIsDebugBuild(bool value);

signals:
    void actionsChanged();
    void navigationChanged();
    void sessionChanged();
    void fileSystemControllerChanged();
    void statusChanged();
    void storageControllerChanged();
    void annualControllerChanged();
    void actorControllerChanged();
    void propertyControllerChanged();
    void contractControllerChanged();
    void statementControllerChanged();
    void transactionControllerChanged();
    void draftControllerChanged();
    void analysisControllerChanged();
    void exportControllerChanged();
    void importControllerChanged();
    void languageControllerChanged();
    void settingsControllerChanged();
    void isDebugBuildChanged();

private:
    ui::Actions* actions_ = nullptr;
    ui::NavigationState* navigation_ = nullptr;
    ui::StateFacade* session_ = nullptr;
    ui::FileSystemController* fileSystemController_ = nullptr;
    ui::StatusState* status_ = nullptr;
    ui::StorageController* storageController_ = nullptr;
    ui::AnnualController* annualController_ = nullptr;
    ui::ActorController* actorController_ = nullptr;
    ui::PropertyController* propertyController_ = nullptr;
    ui::ContractController* contractController_ = nullptr;
    ui::StatementController* statementController_ = nullptr;
    ui::TransactionController* transactionController_ = nullptr;
    ui::DraftController* draftController_ = nullptr;
    ui::AnalysisController* analysisController_ = nullptr;
    ui::ExportController* exportController_ = nullptr;
    ui::ImportController* importController_ = nullptr;
    ui::LanguageController* languageController_ = nullptr;
    ui::SettingsController* settingsController_ = nullptr;
    bool isDebugBuild_ = false;
};

} // namespace ui::bootstrap
