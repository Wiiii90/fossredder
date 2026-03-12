/**
 * @file ui/include/ui/state/SessionSelection.h
 * @brief Declares the selection state wrapper synchronized with session models.
 */

#pragma once

#include <QObject>
#include <QVariant>

#include "ui/state/SelectionState.h"
#include "ui/state/SessionModels.h"

namespace ui {

/**
 * @brief Tracks the currently selected entities across all UI model collections.
 */
class SessionSelection : public QObject {
    Q_OBJECT
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

public:
    /** @brief Creates the selection state wrapper for the supplied session models. */
    explicit SessionSelection(SessionModels& models, QObject* parent = nullptr);

    /** @brief Refreshes selection objects after the underlying models were reloaded. */
    void loadFromState();

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

    QVariant lastAnalysisResult() const { return lastAnalysisResult_; }
    /** @brief Stores the latest analysis payload tied to the active analysis selection. */
    void setLastAnalysisResult(const QVariant& value);

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();
    void selectedStatementIdChanged();
    void selectedTransactionIdChanged();
    void selectedAnalysisIdChanged();
    void selectedAnnualIdChanged();
    void lastAnalysisResultChanged();

private:
    void bindModelSignals();
    void refreshSelections();
    void clearAnalysisResult();

    SessionModels& models_;
    SelectionState state_;
    QVariant lastAnalysisResult_;
};

}
