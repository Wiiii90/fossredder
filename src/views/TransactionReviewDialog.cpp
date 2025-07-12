#include "views/TransactionReviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

TransactionReviewDialog::TransactionReviewDialog(const QVector<Transaction>& transactions, QWidget* parent)
    : QDialog(parent), currentIndex_(0), transactions_(transactions)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    detailsLabel_ = new QLabel(this);
    mainLayout->addWidget(detailsLabel_);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    prevBtn_ = new QPushButton(tr("Previous"), this);
    nextBtn_ = new QPushButton(tr("Next"), this);
    acceptBtn_ = new QPushButton(tr("Accept"), this);
    ignoreBtn_ = new QPushButton(tr("Ignore"), this);

    btnLayout->addWidget(prevBtn_);
    btnLayout->addWidget(nextBtn_);
    btnLayout->addWidget(acceptBtn_);
    btnLayout->addWidget(ignoreBtn_);
    mainLayout->addLayout(btnLayout);

    connect(prevBtn_, &QPushButton::clicked, this, [this]() {
        if (currentIndex_ > 0) { --currentIndex_; updateView(); }
    });
    connect(nextBtn_, &QPushButton::clicked, this, [this]() {
        if (currentIndex_ < transactions_.size() - 1) { ++currentIndex_; updateView(); }
    });
    // Accept/Ignore: implement as needed

    updateView();
}

void TransactionReviewDialog::updateView() {
    const auto& t = transactions_[currentIndex_];
    detailsLabel_->setText(tr("Booking Date: %1\nAmount: %2\nDetails: %3")
        .arg(QString::fromStdString(t.getBookingDate()))
        .arg(t.getAmount())
        .arg(QString::fromStdString(t.details)));
    prevBtn_->setEnabled(currentIndex_ > 0);
    nextBtn_->setEnabled(currentIndex_ < transactions_.size() - 1);
}