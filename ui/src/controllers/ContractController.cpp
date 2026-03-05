#include "ui/controllers/ContractController.h"

#include "ui/controllers/ControllerGuard.h"
#include "ui/controllers/ControllerStrings.h"

#include "core/models/Contract.h"

#include <unordered_set>

namespace ui {

ContractController::ContractController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString ContractController::addContract(const QString& name, const QString& type, const QString& description,
                                        const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::ContractController::addContract")) return {};
    try {
        return QString::fromStdString(core_->addContract(strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds)));
    } catch (...) {
        controllers::guard::reportException("ui::ContractController::addContract");
    }
    return {};
}

void ContractController::updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                        const QStringList& actorIds, const QStringList& propertyIds)
{
    if (!controllers::guard::ensureCore(core_, "ui::ContractController::updateContract")) return;
    try {
        core_->updateContract(id.toStdString(), strings::toStdString(name), strings::toStdString(type), strings::toStdString(description), strings::toStdList(actorIds), strings::toStdList(propertyIds));
    } catch (...) {
        controllers::guard::reportException("ui::ContractController::updateContract");
    }
}

void ContractController::deleteContract(const QString& id)
{
    if (!controllers::guard::ensureCore(core_, "ui::ContractController::deleteContract")) return;
    try {
        core_->deleteContract(id.toStdString());
    } catch (...) {
        controllers::guard::reportException("ui::ContractController::deleteContract");
    }
}

QStringList ContractController::getContractTypes() const
{
    QStringList out;
    if (!controllers::guard::ensureCore(core_, "ui::ContractController::getContractTypes")) return out;
    try {
        std::unordered_set<std::string> seen;
        for (const auto& c : core_->state().contracts) {
            if (!c) continue;
            const std::string type = c->type;
            if (type.empty()) continue;
            if (seen.find(type) != seen.end()) continue;
            seen.insert(type);
            out.push_back(QString::fromStdString(type));
        }
    } catch (...) {
        controllers::guard::reportException("ui::ContractController::getContractTypes");
    }
    return out;
}

}
