#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/controllers/AppStateController.h"

namespace ui {

class ContractController : public QObject {
    Q_OBJECT
public:
    explicit ContractController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds);
    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds);
    Q_INVOKABLE void deleteContract(const QString& id);

private:
    AppStateController* core_ = nullptr;
};

}
