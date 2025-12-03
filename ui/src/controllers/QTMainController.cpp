#include "ui/controllers/QTMainController.h"

namespace ui {

QTMainController::QTMainController(QObject* parent)
    : QObject(parent)
{
}

ContextLevel QTMainController::compute_lca(const QList<QString>& ids) const {
    if (ids.isEmpty()) return ContextLevel::Root;

    bool any_year = false;
    bool any_stmt = false;
    bool any_bg = false;
    bool any_tx = false;

    for (const QString& id : ids) {
        if (id.startsWith("year-")) any_year = true;
        if (id.startsWith("stmt-")) any_stmt = true;
        if (id.startsWith("bg-")) any_bg = true;
        if (id.startsWith("tx-")) any_tx = true;
    }

    // Prefer higher-level context if mixed selection includes higher levels (year > stmt > bg > tx)
    if (any_year) return ContextLevel::Annual;
    if (any_stmt) return ContextLevel::Statement;
    if (any_bg) return ContextLevel::BookingGroup;
    if (any_tx) return ContextLevel::Transaction;

    return ContextLevel::Root;
}

}
