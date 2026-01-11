#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class EditSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sessionId READ sessionId CONSTANT)
    Q_PROPERTY(QString txId READ txId CONSTANT)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(QString bookingDate READ bookingDate WRITE setBookingDate NOTIFY changed)
    Q_PROPERTY(QString valuta READ valuta WRITE setValuta NOTIFY changed)
    Q_PROPERTY(double amount READ amount WRITE setAmount NOTIFY changed)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY changed)
    Q_PROPERTY(QString statementId READ statementId WRITE setStatementId NOTIFY changed)
    Q_PROPERTY(QString actorId READ actorId WRITE setActorId NOTIFY changed)
    Q_PROPERTY(QString actorProposal READ actorProposal WRITE setActorProposal NOTIFY changed)
    Q_PROPERTY(QStringList propertyIds READ propertyIds WRITE setPropertyIds NOTIFY changed)
    Q_PROPERTY(bool allocatable READ allocatable WRITE setAllocatable NOTIFY changed)
    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY changed)

    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY dirtyChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(int expectedVersion READ expectedVersion WRITE setExpectedVersion NOTIFY changed)

public:
    explicit EditSession(QObject* parent = nullptr);

    QString sessionId() const;
    QString txId() const;

    QString name() const;
    QString bookingDate() const;
    QString valuta() const;
    double amount() const;
    QString description() const;
    QString statementId() const;
    QString actorId() const;
    QString actorProposal() const;
    QStringList propertyIds() const;
    bool allocatable() const;
    int status() const;

    bool hasUnsavedChanges() const;
    bool saving() const;
    int expectedVersion() const;

    Q_INVOKABLE void apply();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE bool hasPropertyId(const QString& id) const;
    Q_INVOKABLE QStringList getPropertyIds() const;
    Q_INVOKABLE void setSaving(bool s);
    Q_INVOKABLE void clearDirty();

    // Populate session fields from a snapshot without marking the session dirty
    void loadSnapshot(const QString& name,
                      const QString& bookingDate,
                      double amount,
                      const QString& description,
                      const QString& statementId,
                      const QString& actorId,
                      const QString& actorProposal,
                      const QStringList& propertyIds,
                      bool allocatable,
                      int status,
                      int expectedVersion);

    // Initialize session for a given transaction id and snapshot data
    void startSession(const QString& txId);
    void resetFromSnapshot();

    void setExpectedVersion(int v);

signals:
    void changed();
    void dirtyChanged();
    void savingChanged();
    void saved(const QString& sessionId, bool success);

    // Emitted when QML requests an apply; controller should connect and perform apply(sessionId)
    void requestApply(const QString& sessionId);

public slots:
    void setName(const QString& v);
    void setBookingDate(const QString& v);
    void setValuta(const QString& v);
    void setAmount(double v);
    void setDescription(const QString& v);
    void setStatementId(const QString& v);
    void setActorId(const QString& v);
    void setActorProposal(const QString& v);
    void setPropertyIds(const QStringList& v);
    void setAllocatable(bool v);
    void setStatus(int v);

private:
    QString sessionId_;
    QString txId_;

    QString name_;
    QString bookingDate_;
    QString valuta_;
    double amount_ = 0.0;
    QString description_;
    QString statementId_;
    QString actorId_;
    QString actorProposal_;
    QStringList propertyIds_;
    bool allocatable_ = false;
    int status_ = 0;

    bool dirty_ = false;
    bool saving_ = false;
    int expectedVersion_ = 0;

    void markDirty();
};
