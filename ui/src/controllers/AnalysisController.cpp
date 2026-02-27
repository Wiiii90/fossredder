#include "ui/controllers/AnalysisController.h"

#include <QUuid>
#include <QVariant>

#include "ui/controllers/ControllerStrings.h"
#include "core/analysis/AnalysisController.h"
#include "core/analysis/Filter.h"
#include "core/models/Analysis.h"
#include "core/models/Contract.h"
#include "core/models/Transaction.h"

#include <unordered_set>

namespace ui {

AnalysisController::AnalysisController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QStringList AnalysisController::getContractTypes() const
{
    QStringList out;
    if (!core_) return out;
    std::unordered_set<std::string> seen;
    for (const auto& c : core_->state().contracts) {
        if (!c) continue;
        const std::string t = c->type;
        if (t.empty()) continue;
        if (seen.find(t) != seen.end()) continue;
        seen.insert(t);
        out.push_back(QString::fromStdString(t));
    }
    return out;
}

QString AnalysisController::addAnalysis(const QString& name, const QString& type, const QString& configJson, const QString& filterSpec)
{
    if (!core_) return {};
    auto analysis = std::make_shared<Analysis>();
    analysis->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    analysis->name = strings::toStdString(name);
    analysis->type = strings::toStdString(type);
    analysis->configJson = strings::toStdString(configJson);
    analysis->filterSpec = strings::toStdString(filterSpec);
    core_->mutableState().analyses.push_back(analysis);
    core_->notifyState();
    core_->commit();
    return QString::fromStdString(analysis->id);
}

QVariantMap AnalysisController::computeAnalysis(const QString& analysisId, const QString& filterSpec) const
{
    QVariantMap out;
    if (!core_) return out;

    const std::string aid = analysisId.toStdString();
    const ::AnalysisController ctrl;

    for (const auto& a : core_->state().analyses) {
        if (!a) continue;
        if (a->id != aid) continue;

        const std::string effectiveFilter = filterSpec.isEmpty() ? a->filterSpec : filterSpec.toStdString();
        const auto res = ctrl.computeAnalysis(*a, core_->state(), effectiveFilter);

        QVariantMap metrics;
        for (const auto& kv : res.metrics) metrics.insert(QString::fromStdString(kv.first), kv.second);

        QVariantList table;
        for (const auto& row : res.table) {
            QVariantList r;
            for (const auto& col : row) r.push_back(QString::fromStdString(col));
            table.push_back(r);
        }

        QVariantList artifacts;
        for (const auto& art : res.artifacts) artifacts.push_back(QString::fromStdString(art));

        out["metrics"] = metrics;
        out["table"] = table;

        std::string outType = a->type;
        std::string outConfig = a->configJson;
        if (outType == "plot" && !outConfig.empty()) {
            const std::string key = "\"plotType\"";
            auto pos = outConfig.find(key);
            if (pos != std::string::npos) {
                auto colon = outConfig.find(':', pos + key.size());
                if (colon != std::string::npos) {
                    auto firstQuote = outConfig.find('"', colon + 1);
                    if (firstQuote != std::string::npos) {
                        auto secondQuote = outConfig.find('"', firstQuote + 1);
                        if (secondQuote != std::string::npos && secondQuote > firstQuote) {
                            outType = outConfig.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                        }
                    }
                }
            }
        }

        out["type"] = QString::fromStdString(outType);
        out["config"] = QString::fromStdString(outConfig);

        QVariantList txlist;
        Filter f = parseFilterSpec(effectiveFilter);
        for (const auto& tptr : core_->state().transactions) {
            if (!tptr) continue;
            bool match = true;
            if (!effectiveFilter.empty()) match = f.matches(tptr, core_->state());
            if (!match) continue;
            QVariantMap tm;
            tm["id"] = QString::fromStdString(tptr->id);
            tm["name"] = QString::fromStdString(tptr->name);
            tm["date"] = QString::fromStdString(tptr->bookingDate);
            tm["amount"] = tptr->amount;
            tm["contractId"] = QString::fromStdString(tptr->contractId);

            std::string ctype = "unassigned";
            if (!tptr->contractId.empty()) {
                for (const auto& cptr : core_->state().contracts) {
                    if (!cptr) continue;
                    if (cptr->id == tptr->contractId) { ctype = cptr->type; break; }
                }
            }
            tm["contractType"] = QString::fromStdString(ctype);
            txlist.push_back(tm);
        }

        out["transactions"] = txlist;
        out["artifacts"] = artifacts;
        out["generatedAt"] = QString::fromStdString(res.generatedAt);
        return out;
    }

    return out;
}

}
