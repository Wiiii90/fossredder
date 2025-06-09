#include "views/ManageCategoriesWidget.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageCategoriesWidget::ManageCategoriesWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("Categories (e.g. Rent, Utilities, etc.)"), this);
    categoriesList_ = new QListWidget(this);

    QHBoxLayout* crudLayout = new QHBoxLayout();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    crudLayout->addWidget(addBtn_);
    crudLayout->addWidget(editBtn_);
    crudLayout->addWidget(deleteBtn_);
    crudLayout->addStretch();

    mainLayout->addWidget(label);
    mainLayout->addWidget(categoriesList_, 1);
    mainLayout->addLayout(crudLayout);

    setLayout(mainLayout);
}