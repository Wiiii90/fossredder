/**
 * @file ui/include/ui/state/StateFacadeProjection.h
 * @brief Declares helpers that project session store data into QML-friendly rows.
 */

#pragma once

#include <QVariantList>

#include <QString>

namespace ui {

class SessionStore;

QVariantList buildStatementTransactionIds(const SessionStore& session, const QString& statementId);
QVariantList buildActorRows(const SessionStore& session);
QVariantList buildPropertyRows(const SessionStore& session);
QVariantList buildContractRows(const SessionStore& session);
QVariantList buildAnalysisRows(const SessionStore& session);
QVariantList buildStatementRows(const SessionStore& session);
QVariantList buildStatementTransactionRows(const SessionStore& session, const QString& statementId);

} // namespace ui
