#include "views/ManageActorsWidget.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageActorsWidget::ManageActorsWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Actors (Counterparties, Payees, Partners, etc.)"), this);
    actorsList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(actorsList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);
}