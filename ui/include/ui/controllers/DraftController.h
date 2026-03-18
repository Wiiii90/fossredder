/**
 * @file ui/include/ui/controllers/DraftController.h
 * @brief Declares the UI controller that finalizes imported statement drafts.
 */

#pragma once

#include <QObject>
#include <QString>

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

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
