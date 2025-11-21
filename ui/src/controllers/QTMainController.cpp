#include "ui/controllers/QTMainController.h"

namespace ui {

QTMainController::QTMainController(QObject* parent)
    : QObject(parent)
{
}

void QTMainController::onTreeSelectionChanged(ContextLevel level, const QList<QString>& ids) {
    // here we receive already computed level from widget
    emit contextChanged(level, ids);
}

ContextLevel QTMainController::compute_lca(const QList<QString>& ids) const {
    if (ids.isEmpty()) return ContextLevel::Root;
    // primitive heuristic: id prefix parsing
    bool all_stmt = true;
    bool all_bg = true;
    bool all_tx = true;
    for (const QString& id : ids) {
        if (!id.startsWith("stmt-")) all_stmt = false;
        if (!id.startsWith("bg-")) all_bg = false;
        if (!id.startsWith("tx-")) all_tx = false;
    }
    if (all_tx) return ContextLevel::Transaction;
    if (all_bg) return ContextLevel::BookingGroup;
    if (all_stmt) return ContextLevel::Statement;
    // mixed: pick highest common ancestor - crude fallback
    return ContextLevel::BookingGroup;
}

}
