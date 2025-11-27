#include "ui/views/StatementsView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

StatementsView::StatementsView(QWidget* parent)
    : BaseView(parent)
{
    QVBoxLayout* l = new QVBoxLayout(content_area_);
    QLabel* h = new QLabel(tr("Statements View Content"), content_area_);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void StatementsView::setContext(const QList<QString>& ids, ContextLevel level) {
    Q_UNUSED(ids);
    Q_UNUSED(level);
    set_actions({tr("Import"), tr("New"), tr("Edit"), tr("Delete"), tr("Analyze"), tr("Settings")});
}

}