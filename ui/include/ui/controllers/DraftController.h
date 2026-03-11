#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/AppStateController.h"

namespace ui {

class StatementDraft;

class DraftController : public QObject {
    Q_OBJECT
public:
    explicit DraftController(core::controllers::AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);

private:
    core::controllers::AppStateController* core_ = nullptr;
};

}
