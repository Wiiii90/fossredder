#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/AppStateController.h"

namespace ui {

class StatementController : public QObject {
    Q_OBJECT
public:
    explicit StatementController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addStatement(const QString& name);
    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);
    Q_INVOKABLE void deleteStatement(const QString& id);

private:
    AppStateController* core_ = nullptr;
};

}
