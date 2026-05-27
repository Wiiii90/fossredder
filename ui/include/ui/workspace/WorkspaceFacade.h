/**
 * @file ui/include/ui/workspace/WorkspaceFacade.h
 * @brief Declares the aggregated workspace and selection facade exposed to QML.
 */

#pragma once

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <qqmlintegration.h>

#include <memory>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/application/workspace/WorkspaceSessionState.h"
#include "ui/viewmodels/catalog/ActorListModel.h"
#include "ui/viewmodels/reporting/AnalysisListModel.h"
#include "ui/viewmodels/reporting/AnnualListModel.h"
#include "ui/viewmodels/catalog/ContractListModel.h"
#include "ui/viewmodels/catalog/PropertyListModel.h"
#include "ui/viewmodels/booking/StatementListModel.h"
#include "ui/viewmodels/booking/TransactionListModel.h"
#include "ui/viewmodels/booking/TransactionFilterModel.h"
#include "ui/state/session/WorkspaceSessionSelection.h"
#include "ui/state/session/WorkspaceSessionState.h"

namespace ui {

/**
 * @brief Aggregates workspace data and current selection state for the UI.
 */
class WorkspaceFacade : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(WorkspaceFacade)
    QML_UNCREATABLE("WorkspaceFacade is provided by the application context")
    Q_PROPERTY(SessionState* session READ session CONSTANT)
    Q_PROPERTY(SessionSelection* selection READ selection CONSTANT)
    Q_PROPERTY(ActorList* actors READ actors CONSTANT)
    Q_PROPERTY(PropertyList* properties READ properties CONSTANT)
    Q_PROPERTY(ContractList* contracts READ contracts CONSTANT)
    Q_PROPERTY(StatementList* statements READ statements CONSTANT)
    Q_PROPERTY(TransactionList* transactions READ transactions CONSTANT)
    Q_PROPERTY(AnalysisList* analyses READ analyses CONSTANT)
    Q_PROPERTY(AnnualList* annuals READ annuals CONSTANT)

    Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE setSelectedActorId NOTIFY selectedActorIdChanged)
    Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
    Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE setSelectedContractId NOTIFY selectedContractIdChanged)
    Q_PROPERTY(QString selectedStatementId READ selectedStatementId WRITE setSelectedStatementId NOTIFY selectedStatementIdChanged)
    Q_PROPERTY(QString selectedTransactionId READ selectedTransactionId WRITE setSelectedTransactionId NOTIFY selectedTransactionIdChanged)
    Q_PROPERTY(QString selectedAnalysisId READ selectedAnalysisId WRITE setSelectedAnalysisId NOTIFY selectedAnalysisIdChanged)
    Q_PROPERTY(QString selectedAnnualId READ selectedAnnualId WRITE setSelectedAnnualId NOTIFY selectedAnnualIdChanged)

    Q_PROPERTY(ActorSelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(PropertySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(ContractSelection* selectedContract READ selectedContract CONSTANT)
    Q_PROPERTY(StatementSelection* selectedStatement READ selectedStatement CONSTANT)
    Q_PROPERTY(TransactionSelection* selectedTransaction READ selectedTransaction CONSTANT)
    Q_PROPERTY(AnalysisSelection* selectedAnalysis READ selectedAnalysis CONSTANT)
    Q_PROPERTY(AnnualSelection* selectedAnnual READ selectedAnnual CONSTANT)

    Q_PROPERTY(QVariant lastAnalysisResult READ lastAnalysisResult WRITE setLastAnalysisResult NOTIFY lastAnalysisResultChanged)
    Q_PROPERTY(int dataRevision READ dataRevision NOTIFY dataRevisionChanged)

public:
    /** @brief Creates the facade and its owned workspace state objects. */
    explicit WorkspaceFacade(QObject* parent = nullptr);
    /** @brief Creates the facade and binds it to the application workspace facade. */
    explicit WorkspaceFacade(core::application::WorkspaceFacade* coreFacade, QObject* parent = nullptr);

    /** @brief Returns the session store that owns UI model collections and metrics. */
    SessionState* session() noexcept;
    /** @brief Returns the selection state synchronized with the current session models. */
    SessionSelection* selection() noexcept;

    ActorList* actors() noexcept;
    PropertyList* properties() noexcept;
    ContractList* contracts() noexcept;
    StatementList* statements() noexcept;
    TransactionList* transactions() noexcept;
    AnalysisList* analyses() noexcept;
    AnnualList* annuals() noexcept;

    /** @brief Loads the UI session from the supplied application state snapshot. */
    void loadFromState(const core::domain::catalog::WorkspaceCatalog& state);
    /** @brief Loads the UI session from the full application workspace state. */
    void loadFromState(const core::application::workspace::WorkspaceSessionState& state);
    /** @brief Binds the facade to the application workspace facade used for writes and storage. */
    void setCoreFacade(core::application::WorkspaceFacade* coreFacade) noexcept;
    /** @brief Returns the bound application workspace facade, when available. */
    core::application::WorkspaceFacade* coreFacade() const noexcept;

    Q_INVOKABLE QString currentPath() const;
    Q_INVOKABLE void newFile(const QString& path);
    Q_INVOKABLE void openFile(const QString& path);
    Q_INVOKABLE void saveFile();
    Q_INVOKABLE void saveFileAs(const QString& path);

    Q_INVOKABLE QString addActor(const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE QString saveActor(const QString& id, const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE void deleteActor(const QString& id);

    Q_INVOKABLE QString addProperty(const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE QString saveProperty(const QString& id, const QString& name, const QStringList& aliases = {}, const QStringList& contractIds = {});
    Q_INVOKABLE void deleteProperty(const QString& id);

    Q_INVOKABLE QString addContract(const QString& name,
                                    const QString& type,
                                    const QStringList& actorIds = {},
                                    const QStringList& propertyIds = {},
                                    const QStringList& aliases = {},
                                    const QString& allocatableMode = QStringLiteral("mixed"));
    Q_INVOKABLE void updateContract(const QString& id,
                                    const QString& name,
                                    const QString& type,
                                    const QStringList& actorIds = {},
                                    const QStringList& propertyIds = {},
                                    const QStringList& aliases = {},
                                    const QString& allocatableMode = QStringLiteral("mixed"));
    Q_INVOKABLE QString saveContract(const QString& id,
                                     const QString& name,
                                     const QString& type,
                                     const QStringList& actorIds = {},
                                     const QStringList& propertyIds = {},
                                     const QStringList& aliases = {},
                                     const QString& allocatableMode = QStringLiteral("mixed"));
    Q_INVOKABLE void deleteContract(const QString& id);

    Q_INVOKABLE QString addStatement(const QString& name);
    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);
    Q_INVOKABLE QString saveStatement(const QString& id, const QString& name);
    Q_INVOKABLE void deleteStatement(const QString& id);

    Q_INVOKABLE QString addTransaction(const QString& name,
                                       const QString& bookingDate,
                                       const QString& valuta,
                                       double amount,
                                       const QString& statementId,
                                       int status = 0,
                                       const QString& actorId = QString(),
                                       const QString& contractId = QString(),
                                       bool allocatable = false,
                                       const QStringList& propertyIds = {});
    Q_INVOKABLE QString insertTransactionAfter(const QString& afterTransactionId,
                                               const QString& name,
                                               const QString& bookingDate,
                                               const QString& valuta,
                                               double amount,
                                               const QString& statementId,
                                               int status = 0,
                                               const QString& actorId = QString(),
                                               const QString& contractId = QString(),
                                               bool allocatable = false,
                                               const QStringList& propertyIds = {});
    Q_INVOKABLE void updateTransaction(const QString& id,
                                       const QString& name,
                                       const QString& bookingDate,
                                       const QString& valuta,
                                       double amount,
                                       const QString& statementId,
                                       int status,
                                       const QString& actorId,
                                       const QString& contractId,
                                       bool allocatable,
                                       const QStringList& propertyIds);
    Q_INVOKABLE QString saveTransaction(const QString& id,
                                        const QString& name,
                                        const QString& bookingDate,
                                        const QString& valuta,
                                        double amount,
                                        const QString& statementId,
                                        int status,
                                        const QString& actorId,
                                        const QString& contractId,
                                        bool allocatable,
                                        const QStringList& propertyIds);
    Q_INVOKABLE void deleteTransaction(const QString& id);

    Q_INVOKABLE QString addAnalysis(const QString& name,
                                    const QString& type,
                                    const QString& configJson,
                                    const QString& filterSpec,
                                    const QString& exportFormat,
                                    bool includeCalculationAdjustments,
                                    const QString& exportStateJson,
                                    const QString& snapshotTransactionsJson);
    Q_INVOKABLE void updateAnalysis(const QString& id,
                                    const QString& name,
                                    const QString& type,
                                    const QString& configJson,
                                    const QString& filterSpec,
                                    const QString& exportFormat,
                                    bool includeCalculationAdjustments,
                                    const QString& exportStateJson,
                                    const QString& snapshotTransactionsJson);
    Q_INVOKABLE void deleteAnalysis(const QString& id);

    Q_INVOKABLE QString addAnnual(const QString& name, int year, const QStringList& analysisIds = {});
    Q_INVOKABLE void updateAnnual(const QString& id, const QString& name, int year, const QStringList& analysisIds = {});
    Q_INVOKABLE QString saveAnnual(const QString& id, const QString& name, int year, const QStringList& analysisIds = {});
    Q_INVOKABLE void deleteAnnual(const QString& id);

    QString selectedActorId() const;
    QString selectedPropertyId() const;
    QString selectedContractId() const;
    QString selectedStatementId() const;
    QString selectedTransactionId() const;
    QString selectedAnalysisId() const;
    QString selectedAnnualId() const;

    void setSelectedActorId(const QString& id);
    void setSelectedPropertyId(const QString& id);
    void setSelectedContractId(const QString& id);
    void setSelectedStatementId(const QString& id);
    void setSelectedTransactionId(const QString& id);
    void setSelectedAnalysisId(const QString& id);
    void setSelectedAnnualId(const QString& id);

    ActorSelection* selectedActor();
    PropertySelection* selectedProperty();
    ContractSelection* selectedContract();
    StatementSelection* selectedStatement();
    TransactionSelection* selectedTransaction();
    AnalysisSelection* selectedAnalysis();
    AnnualSelection* selectedAnnual();

    /** @brief Returns transaction ids that belong to the given statement. */
    Q_INVOKABLE QVariantList statementTransactionIds(const QString& statementId) const;
    Q_INVOKABLE QVariantList actorRows() const;
    Q_INVOKABLE QVariantList propertyRows() const;
    Q_INVOKABLE QVariantList contractRows() const;
    Q_INVOKABLE QVariantList analysisRows() const;
    Q_INVOKABLE QVariantList annualRows() const;
    Q_INVOKABLE QVariantList statementRows() const;
    Q_INVOKABLE QVariantList statementTransactionRows(const QString& statementId) const;
    Q_INVOKABLE QVariantList transactionRows() const;
    Q_INVOKABLE QVariantMap transaction(const QString& id) const;
    Q_INVOKABLE QVariantMap annual(const QString& id) const;
    Q_INVOKABLE QVariantMap annualResultState(const QString& annualId) const;
    Q_INVOKABLE QVariantMap annualResultStatePreview(const QString& annualId,
                                                    const QVariant& selectedIds,
                                                    int year) const;
    Q_INVOKABLE QString analysisIdFromRow(const QVariant& row) const;
    Q_INVOKABLE QStringList normalizeAnalysisIds(const QVariant& values) const;
    Q_INVOKABLE QVariantList assignedAnnualAnalysisRows(const QVariantList& allRows, const QVariant& selectedIds) const;
    Q_INVOKABLE QVariantList availableAnnualAnalysisRows(const QVariantList& allRows, const QVariant& selectedIds) const;
    /** @brief Returns a live filter over transactions that belong to the given statement. */
    Q_INVOKABLE TransactionFilter* statementTransactions(const QString& statementId);
    /** @brief Returns a live filter over transactions assigned to the given property. */
    Q_INVOKABLE TransactionFilter* propertyTransactions(const QString& propertyId);

    /** @brief Applies deletion side effects from the domain layer to UI state. */
    Q_INVOKABLE void applyDeletionImpact(const core::domain::DeletionImpact& impact);
    /** @brief Updates property assignments for a transaction without a full reload. */
    Q_INVOKABLE void setTransactionPropertyIdsImmediate(const QString& txId, const QStringList& propertyIds);

    QVariant lastAnalysisResult() const { return selection_->lastAnalysisResult(); }
    void setLastAnalysisResult(const QVariant& value) { selection_->setLastAnalysisResult(value); }
    int dataRevision() const noexcept { return dataRevision_; }

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();
    void selectedStatementIdChanged();
    void selectedTransactionIdChanged();
    void selectedAnalysisIdChanged();
    void selectedAnnualIdChanged();
    void lastAnalysisResultChanged();
    void dataRevisionChanged();
    void operationSucceeded(const QString& operation);
    void operationFailed(const QString& operation, const QString& error);

private:
    void bumpDataRevision();

    std::unique_ptr<SessionState> session_;
    std::unique_ptr<SessionSelection> selection_;
    core::application::WorkspaceFacade* coreFacade_ = nullptr;
    int dataRevision_ = 0;
};

}
