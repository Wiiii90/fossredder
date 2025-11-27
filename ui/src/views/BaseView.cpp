#include "ui/views/BaseView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QColor>

namespace ui {

BaseView::BaseView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* main = new QVBoxLayout(this);
    QHBoxLayout* header = new QHBoxLayout();

    breadcrumb_ = new QLabel(tr(""), this);
    breadcrumb_->setObjectName("breadcrumb");
    action_area_ = new QWidget(this);
    action_area_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    header->addWidget(breadcrumb_);
    header->addStretch();
    header->addWidget(action_area_);

    content_area_ = new QWidget(this);
    content_area_->setObjectName("content_area");
    content_area_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    main->addLayout(header);
    main->addWidget(content_area_);
}

void BaseView::setContext(const QList<QString>& /*ids*/, ContextLevel /*level*/) {
}

void BaseView::set_actions(const QStringList& labels) {
    emit actionsChanged(labels);
}

void BaseView::clear_actions() {
    emit actionsChanged(QStringList());
}

}
