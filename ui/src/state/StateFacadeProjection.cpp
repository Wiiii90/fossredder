/**
 * @file ui/src/state/StateFacadeProjection.cpp
 * @brief Implements helpers that project session store data into QML-friendly rows.
 */

#include "ui/state/StateFacadeProjection.h"

#include "ui/payload/ProjectionConverters.h"
#include "ui/state/SessionStore.h"

#include <QStringList>
#include <QVariantMap>

namespace ui {

QVariantList buildStatementTransactionIds(const SessionStore& session, const QString& statementId)
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) == statementId) {
            out.push_back(QString::fromStdString(transaction->id));
        }
    }
    return out;
}

QVariantList buildActorRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& actor : session.models().actors().actors()) {
        if (!actor) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(actor->id));
        row.insert("name", QString::fromStdString(actor->name));
        row.insert("type", QString::fromStdString(actor->type));
        row.insert("display", actor->type.empty()
                                  ? QString::fromStdString(actor->name)
                                  : QString::fromStdString(actor->name) + QStringLiteral(" — ") + QString::fromStdString(actor->type));
        row.insert("aliases", payload::projection::toQStringList(actor->aliases));
        out.push_back(row);
    }
    return out;
}

QVariantList buildPropertyRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& property : session.models().properties().properties()) {
        if (!property) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(property->id));
        row.insert("name", QString::fromStdString(property->name));
        row.insert("display", property->address.empty()
                                  ? QString::fromStdString(property->name)
                                  : QString::fromStdString(property->name) + QStringLiteral(" — ") + QString::fromStdString(property->address));
        row.insert("aliases", payload::projection::toQStringList(property->aliases));
        out.push_back(row);
    }
    return out;
}

QVariantList buildContractRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& contract : session.models().contracts().contracts()) {
        if (!contract) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(contract->id));
        row.insert("name", QString::fromStdString(contract->name));
        row.insert("type", QString::fromStdString(contract->type));
        row.insert("display", QString::fromStdString(contract->name));
        row.insert("aliases", payload::projection::toQStringList(contract->aliases));
        row.insert("actorIds", payload::projection::toQStringList(contract->actorIds));
        row.insert("propertyIds", payload::projection::toQStringList(contract->propertyIds));
        out.push_back(row);
    }
    return out;
}

QVariantList buildAnalysisRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& analysis : session.models().analyses().analyses()) {
        if (!analysis) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(analysis->id));
        row.insert("name", QString::fromStdString(analysis->name));
        row.insert("type", QString::fromStdString(analysis->type));
        out.push_back(row);
    }
    return out;
}

QVariantList buildStatementRows(const SessionStore& session)
{
    QVariantList out;
    for (const auto& statement : session.models().statements().statements()) {
        if (!statement) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(statement->id));
        row.insert("name", QString::fromStdString(statement->name));
        out.push_back(row);
    }
    return out;
}

QVariantList buildStatementTransactionRows(const SessionStore& session, const QString& statementId)
{
    QVariantList out;
    if (statementId.isEmpty()) return out;

    for (const auto& transaction : session.models().transactions().transactions()) {
        if (!transaction) continue;
        if (QString::fromStdString(transaction->statementId) != statementId) continue;

        QVariantMap row;
        row.insert("id", QString::fromStdString(transaction->id));
        row.insert("name", QString::fromStdString(transaction->name));
        row.insert("bookingDate", QString::fromStdString(transaction->bookingDate));
        out.push_back(row);
    }
    return out;
}

} // namespace ui
