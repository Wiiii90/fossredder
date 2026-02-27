#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "core/controllers/AppStateController.h"

namespace ui {

class AnalysisController : public QObject {
    Q_OBJECT
public:
    explicit AnalysisController(AppStateController* core, QObject* parent = nullptr);

    Q_INVOKABLE QStringList getContractTypes() const;
    Q_INVOKABLE QString addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE QVariantMap computeAnalysis(const QString& analysisId, const QString& filterSpec) const;

private:
    AppStateController* core_ = nullptr;
};

}
