#include "views/DashboardWidget.h"
#include <QVBoxLayout>
#include <QLabel>

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Dashboard / Overview (Statistics, Calendar, Bookings, Plots)"), this));
    setLayout(layout);
}