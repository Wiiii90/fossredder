#include "ui/views/BookingGroupView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

BookingGroupView::BookingGroupView(QWidget* parent)
    : BaseView(parent)
{
    QVBoxLayout* l = new QVBoxLayout(content_area_);
    QLabel* h = new QLabel(tr("Booking Group View Content"), content_area_);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void BookingGroupView::setContext(const QList<QString>& ids, ContextLevel level) {
    Q_UNUSED(ids);
    Q_UNUSED(level);
    set_actions({tr("New"), tr("Edit"), tr("Delete"), tr("Analyze"), tr("Settings")});
}

}