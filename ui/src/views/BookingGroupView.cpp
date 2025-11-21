#include "ui/views/BookingGroupView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

BookingGroupView::BookingGroupView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    QLabel* h = new QLabel(tr("Booking Group View"), this);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void BookingGroupView::setContext(const QList<QString>& ids) {
    Q_UNUSED(ids);
}

}