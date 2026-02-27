#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/controllers/AppStateController.h"

class ActorController : public QObject {
    Q_OBJECT
public:
    explicit ActorController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});
    Q_INVOKABLE void deleteActor(const QString& id);

private:
    AppStateController* core_ = nullptr;
};
