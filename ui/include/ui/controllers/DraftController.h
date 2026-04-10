/**
 * @file ui/include/ui/controllers/DraftController.h
 * @brief Declares the UI controller that finalizes imported statement drafts.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "ui/models/StatementDraft.h"

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes statement-draft finalization to QML.
 */
class DraftController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(DraftController)
    QML_UNCREATABLE("DraftController is provided by the application context")
public:
    /** @brief Create a draft controller bound to the application facade.
     *  @param core Core application facade pointer
     *  @param parent QObject parent
     */
    explicit DraftController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Finalize the current statement draft into persistent core entities.
     *  @param draft Pointer to the UI StatementDraft
     *  @return Identifier of the created statement or empty string on failure
     */
    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);

    /** @brief Build the derived view state for the currently selected draft transaction.
     *  @param draft Pointer to the UI StatementDraft
     *  @return Serialized view state as QVariantMap
     */
    Q_INVOKABLE QVariantMap currentTransactionViewState(StatementDraft* draft) const;

    /** @brief Find a choice row whose text matches the provided search term.
     *  @param rows List of choice rows
     *  @param text Search text
     *  @return Matching row as QVariantMap or empty when not found
     */
    Q_INVOKABLE QVariantMap findChoiceRowByText(const QVariantList& rows, const QString& text) const;

    /** @brief Recompute and apply derived selections for the current draft transaction.
     *  @param draft Pointer to the UI StatementDraft
     */
    Q_INVOKABLE void syncCurrentTransactionDraft(StatementDraft* draft);

    /** @brief Select the current actor choice from a serialized row payload.
     *  @param draft Pointer to the UI StatementDraft
     *  @param row Choice row payload
     */
    Q_INVOKABLE void selectCurrentActorChoice(StatementDraft* draft, const QVariantMap& row);

    /** @brief Select the current contract choice from a serialized row payload.
     *  @param draft Pointer to the UI StatementDraft
     *  @param row Choice row payload
     */
    Q_INVOKABLE void selectCurrentContractChoice(StatementDraft* draft, const QVariantMap& row);

    /** @brief Toggle a property selection on the current draft transaction.
     *  @param draft Pointer to the UI StatementDraft
     *  @param propertyId Property identifier
     *  @param selected Selection state
     */
    Q_INVOKABLE void setCurrentPropertySelected(StatementDraft* draft, const QString& propertyId, bool selected);

    /** @brief Parse and apply an amount edit to the current draft transaction.
     *  @param draft Pointer to the UI StatementDraft
     *  @param text Amount text to parse
     */
    Q_INVOKABLE void updateCurrentAmount(StatementDraft* draft, const QString& text);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
