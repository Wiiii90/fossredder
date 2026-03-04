#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "core/controllers/AppStateController.h"
#include "core/models/Transaction.h"

namespace ui {

class StatementDraft;

class DomainController : public QObject {
    Q_OBJECT
public:
    explicit DomainController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});
    Q_INVOKABLE void deleteActor(const QString& id);

    Q_INVOKABLE QString addAnnual(int year);
    Q_INVOKABLE void updateAnnual(const QString& id, int year);
    Q_INVOKABLE void deleteAnnual(const QString& id);

    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds);
    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds);
    Q_INVOKABLE void deleteContract(const QString& id);

    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void deleteProperty(const QString& id);

    Q_INVOKABLE QString addStatement(const QString& name);
    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);
    Q_INVOKABLE void deleteStatement(const QString& id);

    Q_INVOKABLE QString addTransaction(const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status = static_cast<int>(Transaction::Status::Neutral),
                                       const QString& actorId = QString(),
                                       bool allocatable = false,
                                       const QStringList& propertyIds = {});

    Q_INVOKABLE void updateTransaction(const QString& id,
                                       const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status,
                                       const QString& actorId,
                                       bool allocatable,
                                       const QStringList& propertyIds);

    Q_INVOKABLE void deleteTransaction(const QString& id);

    Q_INVOKABLE QStringList getContractTypes() const;
    Q_INVOKABLE QString addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;

    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);

private:
    AppStateController* core_ = nullptr;
};

}
