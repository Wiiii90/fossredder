#include "ui/windows/Workspace.h"
#include "ui/widgets/BackgroundWidget.h"
#include "ui/widgets/TreeWidget.h"
#include "ui/views/BaseView.h"
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace ui {

Workspace::Workspace(QWidget* parent)
    : QWidget(parent)
{
    // Build central composition
    BackgroundWidget* central = new BackgroundWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0,0,0,0);
    centralLayout->setSpacing(0);

    QSplitter* splitter = new QSplitter(central);
    splitter->setAttribute(Qt::WA_StyledBackground, false);
    splitter->setAutoFillBackground(false);

    m_tree = new TreeWidget(splitter);
    m_stack = new QStackedWidget(splitter);
    m_stack->setAttribute(Qt::WA_StyledBackground, false);
    m_stack->setAutoFillBackground(false);

    splitter->addWidget(m_tree);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    centralLayout->addWidget(splitter);
    central->setLayout(centralLayout);

    // this widget wraps central
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(central);
    setLayout(layout);

    m_central = central;
}

TreeWidget* Workspace::treeWidget() const { return m_tree; }
QWidget* Workspace::centralWidget() const { return m_central; }
QStackedWidget* Workspace::stackedWidget() const { return m_stack; }

void Workspace::setViews(const QList<BaseView*>& views) {
    m_views = views;
    if (!m_stack) return;
    while (m_stack->count() > 0) m_stack->removeWidget(m_stack->widget(0));
    for (BaseView* v : m_views) m_stack->addWidget(v);
}

void Workspace::setCurrentIndex(int idx) {
    if (!m_stack) return;
    m_stack->setCurrentIndex(idx);
}

}
