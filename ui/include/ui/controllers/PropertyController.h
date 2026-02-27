#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/AppStateController.h"

class PropertyController : public QObject {
    Q_OBJECT
public:
    explicit PropertyController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void deleteProperty(const QString& id);

private:
    AppStateController* core_ = nullptr;
};
