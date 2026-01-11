#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class UiEntitySelection : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString name READ name NOTIFY changed)
    Q_PROPERTY(QString type READ type NOTIFY changed)
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
    explicit UiEntitySelection(QObject* parent = nullptr);

    void clear();

    void setActor(const QString& id, const QString& name, const QString& type, const QString& description);
    void setProperty(const QString& id, const QString& name, const QString& address, const QString& description);

    void setStatement(const QString& id, const QString& name);
    void setTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId);
    void setTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, const QString& actorId);
    void setTransaction(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, const QString& actorId, const QString& actorProposal);
    void setTransactionWithProperties(const QString& id, const QString& name, const QString& bookingDate, double amount, const QString& description, const QString& statementId, const QString& actorId, const QString& actorProposal, const QStringList& propertyIds, bool allocatable, const QString& transactionType);

    void setContract(const QString& id, const QString& name, const QString& type, const QString& description);
    void setContract(const QString& id, const QString& name, const QString& type, const QString& description,
                     const QStringList& actorIds, const QStringList& propertyIds);

    QString id() const { return id_; }
    QString name() const { return name_; }
    QString type() const { return type_; }
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
