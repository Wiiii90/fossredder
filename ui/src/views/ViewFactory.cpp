#include "ui/views/ViewFactory.h"
#include "ui/views/AnnualView.h"
#include "ui/views/StatementsView.h"
#include "ui/views/BookingGroupView.h"
#include "ui/views/TransactionView.h"
#include "ui/views/BaseView.h"

#include <QMetaObject>

namespace ui {

QList<BaseView*> ViewFactory::createViews(QWidget* parent) {
    QList<BaseView*> views;
    views.append(new AnnualView(parent));
    views.append(new StatementsView(parent));
    views.append(new BookingGroupView(parent));
    views.append(new TransactionView(parent));
    return views;
}

int ViewFactory::maxSlotsForViews(const QList<BaseView*>& views) {
    int maxSlots = 0;
    for (BaseView* v : views) {
        if (qobject_cast<AnnualView*>(v)) maxSlots = std::max(maxSlots, 6);
        else if (qobject_cast<StatementsView*>(v)) maxSlots = std::max(maxSlots, 6);
        else if (qobject_cast<BookingGroupView*>(v)) maxSlots = std::max(maxSlots, 5);
        else if (qobject_cast<TransactionView*>(v)) maxSlots = std::max(maxSlots, 3);
    }
    return maxSlots;
}

}