#pragma once

#include <QString>

#include "ui/models/ActorList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/ContractList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/AnalysisList.h"
#include "ui/models/AnnualList.h"

namespace ui {

class EntitySelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString type READ type NOTIFY changed)
    Q_PROPERTY(QStringList aliases READ aliases NOTIFY changed)
    Q_PROPERTY(QString address READ address NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QStringList actorIds READ actorIds NOTIFY changed)
    Q_PROPERTY(QStringList propertyIds READ propertyIds NOTIFY changed)
    Q_PROPERTY(QString bookingDate READ bookingDate NOTIFY changed)
    Q_PROPERTY(double amount READ amount NOTIFY changed)
    Q_PROPERTY(bool allocatable READ allocatable NOTIFY changed)
    Q_PROPERTY(QString statementId READ statementId NOTIFY changed)
    Q_PROPERTY(QString actorId READ actorId NOTIFY changed)
    Q_PROPERTY(QString actorProposal READ actorProposal NOTIFY changed)

public:
    explicit EntitySelection(QObject* parent = nullptr);

    void clear();
    void setActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});
    void setProperty(const QString& id, const QString& name, const QString& address, const QString& description);
    void setStatement(const QString& id, const QString& name);
    void setTransaction(const QString& id,
                        const QString& name,
                        const QString& bookingDate,
                        double amount,
                        const QString& description,
                        const QString& statementId,
                        const QString& actorId = QString(),
                        const QString& actorProposal = QString(),
                        const QStringList& propertyIds = {},
                        bool allocatable = false,
                        const QString& transactionType = QString());
    void setContract(const QString& id, const QString& name, const QString& type, const QString& description);
    void setContract(const QString& id, const QString& name, const QString& type, const QString& description,
                     const QStringList& actorIds, const QStringList& propertyIds);
    void setAnalysis(const QString& id, const QString& name, const QString& type, const QString& description = QString());
    void setAnnual(const QString& id, int year);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString type() const { return type_; }
    QStringList aliases() const { return aliases_; }
    QString address() const { return address_; }
    QString description() const { return description_; }
    QStringList actorIds() const { return actorIds_; }
    QStringList propertyIds() const { return propertyIds_; }
    QString bookingDate() const { return bookingDate_; }
    double amount() const { return amount_; }
    bool allocatable() const { return allocatable_; }
    QString statementId() const { return statementId_; }
    QString actorId() const { return actorId_; }
    QString actorProposal() const { return actorProposal_; }

    Q_INVOKABLE void setPropertyIds(const QStringList& ids) { propertyIds_ = ids; emit changed(); }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString type_;
    QStringList aliases_;
    QString address_;
    QString description_;
    QStringList actorIds_;
    QStringList propertyIds_;
    QString bookingDate_;
    double amount_ = 0.0;
    bool allocatable_ = false;
    QString statementId_;
    QString actorId_;
    QString actorProposal_;
};

class SelectionState {
public:
    SelectionState(ActorList& actors,
                   PropertyList& properties,
                   ContractList& contracts,
                   StatementList& statements,
                   TransactionList& transactions,
                   AnalysisList& analyses,
                   AnnualList& annuals,
                   QObject* parent);

    const QString& selectedActorId() const;
    const QString& selectedPropertyId() const;
    const QString& selectedContractId() const;
    const QString& selectedStatementId() const;
    const QString& selectedTransactionId() const;
    const QString& selectedAnalysisId() const;
    const QString& selectedAnnualId() const;

    bool setSelectedActorId(const QString& id);
    bool setSelectedPropertyId(const QString& id);
    bool setSelectedContractId(const QString& id);
    bool setSelectedStatementId(const QString& id);
    bool setSelectedTransactionId(const QString& id);
    bool setSelectedAnalysisId(const QString& id);
    bool setSelectedAnnualId(const QString& id);

    EntitySelection* selectedActor();
    EntitySelection* selectedProperty();
    EntitySelection* selectedContract();
    EntitySelection* selectedStatement();
    EntitySelection* selectedTransaction();
    EntitySelection* selectedAnalysis();
    EntitySelection* selectedAnnual();

    void refreshAll();
    void refreshSelectedTransaction();

    bool clearActorIfSelected(const QString& id);
    bool clearPropertyIfSelected(const QString& id);
    bool clearContractIfSelected(const QString& id);
    bool clearStatementIfSelected(const QString& id);
    bool clearTransactionIfSelected(const QString& id);

private:
    ActorList& actors_;
    PropertyList& properties_;
    ContractList& contracts_;
    StatementList& statements_;
    TransactionList& transactions_;
    AnalysisList& analyses_;
    AnnualList& annuals_;

    QString selectedActorId_;
    QString selectedPropertyId_;
    QString selectedContractId_;
    QString selectedStatementId_;
    QString selectedTransactionId_;
    QString selectedAnalysisId_;
    QString selectedAnnualId_;

    EntitySelection selectedActor_;
    EntitySelection selectedProperty_;
    EntitySelection selectedContract_;
    EntitySelection selectedStatement_;
    EntitySelection selectedTransaction_;
    EntitySelection selectedAnalysis_;
    EntitySelection selectedAnnual_;

    void refreshSelectedActor();
    void refreshSelectedProperty();
    void refreshSelectedContract();
    void refreshSelectedStatement();
    void refreshSelectedAnalysis();
    void refreshSelectedAnnual();
};

}
