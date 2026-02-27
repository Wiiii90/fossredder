#include "ui/controllers/StatementController.h"

#include <QUuid>
#include <algorithm>
#include <memory>

#include "core/models/Statement.h"
#include "core/models/Transaction.h"

namespace {
std::string q2s(const QString& s)
{
    const auto u8 = s.toUtf8();
    return std::string(u8.constData(), static_cast<size_t>(u8.size()));
}
}

namespace ui {

StatementController::StatementController(AppStateController* core, QObject* parent)
    : QObject(parent)
    , core_(core)
{
}

QString StatementController::addStatement(const QString& name)
{
    if (!core_) return {};
    auto s = std::make_shared<Statement>();
    s->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    s->name = q2s(name);
    core_->mutableState().statements.push_back(s);
    core_->notifyState();
    return QString::fromStdString(s->id);
}

void StatementController::updateStatement(const QString& id, const QString& name)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    for (auto& s : core_->mutableState().statements) {
        if (!s || s->id != sid) continue;
        s->name = q2s(name);
        core_->notifyState();
        return;
    }
}

void StatementController::deleteStatement(const QString& id)
{
    if (!core_) return;
    const auto sid = id.toStdString();
    auto& tx = core_->mutableState().transactions;
    tx.erase(std::remove_if(tx.begin(), tx.end(), [&](const auto& t) { return t && t->statementId == sid; }), tx.end());

    auto& st = core_->mutableState().statements;
    st.erase(std::remove_if(st.begin(), st.end(), [&](const auto& s) { return s && s->id == sid; }), st.end());
    core_->notifyState();
}

}
