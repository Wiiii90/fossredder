#include "ui/widgets/ToolBarWidget.h"
#include "ui/actions/ActionRegistry.h"
#include "ui/views/BaseView.h"
#include "ui/views/ViewFactory.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDebug>

namespace ui {

ToolBarWidget::ToolBarWidget(ActionRegistry* registry, QWidget* parent)
    : QWidget(parent), m_registry(registry)
{
    setObjectName("mainToolBarWidget");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4,2,4,2);
    mainLayout->setSpacing(8);

    QWidget* globalArea = new QWidget(this);
    globalArea->setObjectName("toolbarGlobalArea");
    QHBoxLayout* globalLayout = new QHBoxLayout(globalArea);
    globalLayout->setContentsMargins(0,0,0,0);
    globalLayout->setSpacing(6);

    auto makeButton = [&](QAction* act){
        QPushButton* b = new QPushButton(act->text(), globalArea);
        b->setFlat(true);
        b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        b->setMaximumWidth(140);
        connect(b, &QPushButton::clicked, act, &QAction::trigger);
        return b;
    };

    globalLayout->addWidget(makeButton(m_registry->importAction()));
    globalLayout->addWidget(makeButton(m_registry->exportAction()));
    globalLayout->addWidget(makeButton(m_registry->annualAction()));
    globalLayout->addWidget(makeButton(m_registry->actorsAction()));
    globalLayout->addWidget(makeButton(m_registry->propertiesAction()));

    m_view_action_area = new QWidget(this);
    m_view_action_area->setObjectName("toolbarViewActionArea");
    QHBoxLayout* dynLayout = new QHBoxLayout(m_view_action_area);
    dynLayout->setContentsMargins(0,0,0,0);
    dynLayout->setSpacing(6);

    mainLayout->addWidget(globalArea, 1);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_view_action_area, 1);
}

QWidget* ToolBarWidget::viewActionArea() const { return m_view_action_area; }

void ToolBarWidget::registerViews(const QList<BaseView*>& views) {
    m_registered_views = views;
}

void ToolBarWidget::updateViewActions(const QStringList& labels) {
    QLayout* lay = m_view_action_area->layout();
    if (lay) {
        QLayoutItem* it;
        while ((it = lay->takeAt(0)) != nullptr) {
            if (QWidget* w = it->widget()) w->deleteLater();
            delete it;
        }
        delete lay;
    }

    int maxSlots = ViewFactory::maxSlotsForViews(m_registered_views);

    if (maxSlots == 0) return;

    QHBoxLayout* layout = new QHBoxLayout(m_view_action_area);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(6);

    for (int i = 0; i < maxSlots; ++i) {
        if (i < labels.size()) {
            QString lbl = labels.at(i);
            QPushButton* b = new QPushButton(lbl, m_view_action_area);
            b->setFlat(true);
            b->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            b->setMaximumWidth(140);
            connect(b, &QPushButton::clicked, this, [lbl](){ qDebug() << "View action:" << lbl; });
            layout->addWidget(b);
        } else {
            QWidget* placeholder = new QWidget(m_view_action_area);
            placeholder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            placeholder->setMaximumWidth(140);
            layout->addWidget(placeholder);
        }
    }
}

}