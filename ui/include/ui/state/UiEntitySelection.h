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

public:
    explicit UiEntitySelection(QObject* parent = nullptr);

    void clear();

    void setActor(const QString& id, const QString& name, const QString& type, const QString& description);
    void setProperty(const QString& id, const QString& name, const QString& address, const QString& description);
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
};
