#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/controllers/AppStateController.h"

namespace ui {

class ActorController : public QObject {
    Q_OBJECT
public:
    explicit ActorController(core::controllers::AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void deleteActor(const QString& id);

private:
    core::controllers::AppStateController* core_ = nullptr;
};

}
