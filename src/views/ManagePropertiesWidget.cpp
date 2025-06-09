#include "views/ManagePropertiesWidget.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManagePropertiesWidget::ManagePropertiesWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Properties (Buildings, Real Estate, etc.)"), this);
    propertiesList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(propertiesList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);
}