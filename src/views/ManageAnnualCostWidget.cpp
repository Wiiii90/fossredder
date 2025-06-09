#include "views/ManageAnnualCostWidget.h"
#include <QVBoxLayout>
#include <QLabel>

ManageAnnualCostWidget::ManageAnnualCostWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Manage Annual Cost Statements: Generate yearly cost reports from bookings"), this));
    setLayout(layout);
}