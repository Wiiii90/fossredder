#include "ui/views/YearlyStatementView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

YearlyStatementView::YearlyStatementView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    QLabel* h = new QLabel(tr("Yearly Statement View"), this);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void YearlyStatementView::setContext(const QList<QString>& ids) {
    Q_UNUSED(ids);
}

}