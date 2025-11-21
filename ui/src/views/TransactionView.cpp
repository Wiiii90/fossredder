#include "ui/views/TransactionView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

TransactionView::TransactionView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    QLabel* h = new QLabel(tr("Transaction View"), this);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void TransactionView::setContext(const QList<QString>& ids) {
    Q_UNUSED(ids);
}

}