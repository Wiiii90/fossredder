#include "ui/views/AnnualView.h"
#include <QVBoxLayout>
#include <QLabel>

namespace ui {

AnnualView::AnnualView(QWidget* parent)
    : BaseView(parent)
{
    QVBoxLayout* l = new QVBoxLayout(content_area_);
    QLabel* h = new QLabel(tr("Annual View Content"), content_area_);
    h->setAlignment(Qt::AlignCenter);
    l->addWidget(h);
}

void AnnualView::setContext(const QList<QString>& ids, ContextLevel level) {
    Q_UNUSED(ids);
    Q_UNUSED(level);
    set_actions({ tr("Generate"), tr("Export"), tr("Edit"), tr("Clear"), tr("Analyze"), tr("Settings") });
}

}