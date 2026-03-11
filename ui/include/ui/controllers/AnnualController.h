#pragma once

#include <QObject>
#include <QString>

#include "core/controllers/AppStateController.h"

namespace ui {

class AnnualController : public QObject {
    Q_OBJECT
public:
    explicit AnnualController(core::controllers::AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addAnnual(int year);
    Q_INVOKABLE void updateAnnual(const QString& id, int year);
    Q_INVOKABLE void deleteAnnual(const QString& id);

private:
    core::controllers::AppStateController* core_ = nullptr;
};

}
