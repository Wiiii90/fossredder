#include "ui/views/StatementsView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

StatementsView::StatementsView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    QLabel* h = new QLabel(tr("Statements View"), this);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void StatementsView::setContext(const QList<QString>& ids) {
    Q_UNUSED(ids);
}

}