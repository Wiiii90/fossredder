/**
 * @file ui/include/ui/controllers/StatementController.h
 * @brief Declares the UI controller that forwards statement mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "core/ports/workspace/IWorkspaceReader.h"
#include "core/ports/workspace/IWorkspaceWriter.h"

namespace ui {

/**
 * @brief Exposes statement create, update and delete actions to QML.
 */
class StatementController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StatementController)
    QML_UNCREATABLE("StatementController is provided by the application context")
public:
    /** @brief Create a statement controller bound to the application facade. */
    explicit StatementController(core::ports::workspace::IWorkspaceWriter* core, QObject* parent = nullptr);

    /** @brief Return a single statement by identifier.
     *  @param id Statement identifier
     *  @return Serialized statement payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap statement(const QString& id) const;

    /** @brief Return all statements.
     *  @return List of serialized statements as QVariantList
     */
    Q_INVOKABLE QVariantList statements() const;

    /** @brief Create a new statement.
     *  @param name Statement name
     *  @return Identifier of the created statement
     */
    Q_INVOKABLE QString addStatement(const QString& name);

    /** @brief Update an existing statement.
     *  @param id Statement identifier
     *  @param name Statement name
     */
    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);

    /** @brief Delete a statement by identifier.
     *  @param id Statement identifier
     */
    Q_INVOKABLE void deleteStatement(const QString& id);

private:
    core::ports::workspace::IWorkspaceWriter* core_ = nullptr;
    core::ports::workspace::IWorkspaceReader* reader_ = nullptr;
};

} // namespace ui
