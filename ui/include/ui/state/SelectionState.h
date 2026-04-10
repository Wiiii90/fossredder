/**
 * @file ui/include/ui/state/SelectionState.h
 * @brief Declarations for the UI SelectionState component.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>

#include "ui/models/ActorList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/ContractList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"
#include "ui/models/AnalysisList.h"
#include "ui/models/AnnualList.h"

namespace ui {

class ActorSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString type READ type NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QStringList aliases READ aliases NOTIFY changed)

public:
    explicit ActorSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name, QString type, QString description, QStringList aliases);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString type() const { return type_; }
    QString description() const { return description_; }
    QStringList aliases() const { return aliases_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString type_;
    QString description_;
    QStringList aliases_;
};

class PropertySelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString address READ address NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QStringList aliases READ aliases NOTIFY changed)

public:
    explicit PropertySelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name, QString address, QString description, QStringList aliases);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString address() const { return address_; }
    QString description() const { return description_; }
    QStringList aliases() const { return aliases_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString address_;
    QString description_;
    QStringList aliases_;
};

class ContractSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString type READ type NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QStringList actorIds READ actorIds NOTIFY changed)
    Q_PROPERTY(QStringList propertyIds READ propertyIds NOTIFY changed)
    Q_PROPERTY(QStringList aliases READ aliases NOTIFY changed)

public:
    explicit ContractSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name, QString type, QString description, QStringList actorIds, QStringList propertyIds, QStringList aliases);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString type() const { return type_; }
    QString description() const { return description_; }
    QStringList actorIds() const { return actorIds_; }
    QStringList propertyIds() const { return propertyIds_; }
    QStringList aliases() const { return aliases_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString type_;
    QString description_;
    QStringList actorIds_;
    QStringList propertyIds_;
    QStringList aliases_;
};

class StatementSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)

public:
    explicit StatementSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name);

    QString id() const { return id_; }
    QString name() const { return name_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
};

class TransactionSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString bookingDate READ bookingDate NOTIFY changed)
    Q_PROPERTY(double amount READ amount NOTIFY changed)
    Q_PROPERTY(QString description READ description NOTIFY changed)
    Q_PROPERTY(QString statementId READ statementId NOTIFY changed)
    Q_PROPERTY(QString actorId READ actorId NOTIFY changed)
    Q_PROPERTY(QStringList propertyIds READ propertyIds NOTIFY changed)
    Q_PROPERTY(bool allocatable READ allocatable NOTIFY changed)

public:
    explicit TransactionSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id,
             QString name,
             QString bookingDate,
             double amount,
             QString description,
             QString statementId,
             QString actorId,
             QStringList propertyIds,
             bool allocatable);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString bookingDate() const { return bookingDate_; }
    double amount() const { return amount_; }
    QString description() const { return description_; }
    QString statementId() const { return statementId_; }
    QString actorId() const { return actorId_; }
    QStringList propertyIds() const { return propertyIds_; }
    bool allocatable() const { return allocatable_; }

    Q_INVOKABLE void setPropertyIds(const QStringList& ids) {
        if (propertyIds_ == ids) return;
        propertyIds_ = ids;
        emit changed();
    }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString bookingDate_;
    double amount_ = 0.0;
    QString description_;
    QString statementId_;
    QString actorId_;
    QStringList propertyIds_;
    bool allocatable_ = false;
};

class AnalysisSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString type READ type NOTIFY changed)

public:
    explicit AnalysisSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name, QString type);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString type() const { return type_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
    QString type_;
};

class AnnualSelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)

public:
    explicit AnnualSelection(QObject* parent = nullptr);

    void clear();
    void set(QString id, QString name);

    QString id() const { return id_; }
    QString name() const { return name_; }

signals:
    void changed();

private:
    QString id_;
    QString name_;
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
                   QObject* objectParent = nullptr);

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

    ActorSelection* selectedActor();
    PropertySelection* selectedProperty();
    ContractSelection* selectedContract();
    StatementSelection* selectedStatement();
    TransactionSelection* selectedTransaction();
    AnalysisSelection* selectedAnalysis();
    AnnualSelection* selectedAnnual();

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

    std::unique_ptr<ActorSelection> selectedActor_;
    std::unique_ptr<PropertySelection> selectedProperty_;
    std::unique_ptr<ContractSelection> selectedContract_;
    std::unique_ptr<StatementSelection> selectedStatement_;
    std::unique_ptr<TransactionSelection> selectedTransaction_;
    std::unique_ptr<AnalysisSelection> selectedAnalysis_;
    std::unique_ptr<AnnualSelection> selectedAnnual_;

    void refreshSelectedActor();
    void refreshSelectedProperty();
    void refreshSelectedContract();
    void refreshSelectedStatement();
    void refreshSelectedAnalysis();
    void refreshSelectedAnnual();
};

}
