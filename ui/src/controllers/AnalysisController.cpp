#include "ui/controllers/AnalysisController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

#include "core/analysis/AnalysisController.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

namespace ui {

AnalysisController::AnalysisController(AppStateController* core, const ::AnalysisController* analysisController, QObject* parent)
    : QObject(parent)
    , core_(core)
    , analysisController_(analysisController)
{
}

QString AnalysisController::addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec)
{
    if (!controllers::guard::ensureCore(core_, "ui::AnalysisController::addAnalysis")) return {};
    try {
        return QString::fromStdString(core_->addAnalysis(strings::toStdString(name), strings::toStdString(type), strings::toStdString(configJson), strings::toStdString(filterSpec)));
    } catch (...) {
        controllers::guard::reportException("ui::AnalysisController::addAnalysis");
    }
    return {};
}

QVariantMap AnalysisController::computeAnalysis(const QString& analysisId, const QString& filterSpec) const
{
    QVariantMap out;
    if (!controllers::guard::ensureCore(core_, "ui::AnalysisController::computeAnalysis")) return out;
    if (!analysisController_) {
        core::errors::report(core::errors::ErrorSeverity::Warning,
                             core::errors::codes::GenericError,
                             "ui::AnalysisController::computeAnalysis",
                             "AnalysisController is null");
        return out;
    }

    try {
        const auto result = analysisController_->computeAnalysisById(analysisId.toStdString(), core_->state(), filterSpec.toStdString());
        if (!result.found) return out;

        QVariantMap metrics;
        for (const auto& kv : result.metrics) metrics.insert(QString::fromStdString(kv.first), kv.second);

        QVariantList table;
        for (const auto& row : result.table) {
            QVariantList r;
            for (const auto& col : row) r.push_back(QString::fromStdString(col));
            table.push_back(r);
        }

        QVariantList artifacts;
        for (const auto& art : result.artifacts) artifacts.push_back(QString::fromStdString(art));

        QVariantList txlist;
        for (const auto& tx : result.transactions) {
            if (!tx) continue;
            QVariantMap tm;
            tm["id"] = QString::fromStdString(tx->id);
            tm["name"] = QString::fromStdString(tx->name);
            tm["date"] = QString::fromStdString(tx->bookingDate);
            tm["amount"] = tx->amount;
            tm["contractId"] = QString::fromStdString(tx->contractId);
            tm["contractType"] = QString::fromStdString(tx->contract ? tx->contract->type : std::string("unassigned"));
            txlist.push_back(tm);
        }

        out["metrics"] = metrics;
        out["table"] = table;
        out["type"] = QString::fromStdString(result.type);
        out["config"] = QString::fromStdString(result.configJson);
        out["transactions"] = txlist;
        out["artifacts"] = artifacts;
        out["generatedAt"] = QString::fromStdString(result.createdAt);
        return out;
    } catch (...) {
        controllers::guard::reportException("ui::AnalysisController::computeAnalysis");
    }

    return out;
}

}
