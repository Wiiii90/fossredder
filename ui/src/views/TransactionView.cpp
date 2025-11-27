#include "ui/views/TransactionView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

TransactionView::TransactionView(QWidget* parent)
    : BaseView(parent)
{
    QVBoxLayout* l = new QVBoxLayout(content_area_);
    QLabel* h = new QLabel(tr("Transaction View Content"), content_area_);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void TransactionView::setContext(const QList<QString>& ids, ContextLevel level) {
    Q_UNUSED(ids);
    Q_UNUSED(level);
    set_actions({tr("New"), tr("Edit"), tr("Settings")});
}

}