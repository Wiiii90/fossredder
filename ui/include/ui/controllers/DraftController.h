/**
 * @file ui/include/ui/controllers/DraftController.h
 * @brief Declares the UI controller that finalizes imported statement drafts.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace core::application { class AppStateFacade; }

namespace ui {

class StatementDraft;

/**
 * @brief Exposes statement-draft finalization to QML.
 */
class DraftController : public QObject {
    Q_OBJECT
public:
    explicit DraftController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);
    Q_INVOKABLE QVariantMap currentTransactionViewState(StatementDraft* draft) const;
    Q_INVOKABLE QVariantMap findChoiceRowByText(const QVariantList& rows, const QString& text) const;
    Q_INVOKABLE void syncCurrentTransactionDraft(StatementDraft* draft);
    Q_INVOKABLE void selectCurrentActorChoice(StatementDraft* draft, const QVariantMap& row);
    Q_INVOKABLE void selectCurrentContractChoice(StatementDraft* draft, const QVariantMap& row);
    Q_INVOKABLE void setCurrentPropertySelected(StatementDraft* draft, const QString& propertyId, bool selected);
    Q_INVOKABLE void updateCurrentAmount(StatementDraft* draft, const QString& text);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
