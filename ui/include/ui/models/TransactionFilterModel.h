#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class TransactionFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString statementId READ statementId WRITE setStatementId NOTIFY statementIdChanged)

public:
    explicit TransactionFilterModel(QObject* parent = nullptr);

    QString statementId() const { return statementId_; }
    void setStatementId(const QString& id);

signals:
    void statementIdChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString statementId_;
};
