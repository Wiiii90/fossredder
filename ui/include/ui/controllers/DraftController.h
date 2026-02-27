#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/AppStateController.h"

class StatementDraft;

class DraftController : public QObject {
    Q_OBJECT
public:
    explicit DraftController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString finalizeStatementDraft(StatementDraft* draft);

private:
    AppStateController* core_ = nullptr;
};
