#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class TransactionFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString statementId READ statementId WRITE setStatementId NOTIFY statementIdChanged)
    Q_PROPERTY(QString propertyId READ propertyId WRITE setPropertyId NOTIFY propertyIdChanged)

public:
    explicit TransactionFilterModel(QObject* parent = nullptr);

    QString statementId() const { return statementId_; }
    void setStatementId(const QString& id);

    QString propertyId() const { return propertyId_; }
    void setPropertyId(const QString& id);

signals:
    void statementIdChanged();
    void propertyIdChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString statementId_;
    QString propertyId_;
};
