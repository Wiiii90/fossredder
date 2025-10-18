#include "views/TransactionReviewDialog.h"
#include <QFormLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "models/Transaction.h"

TransactionReviewDialog::TransactionReviewDialog(
    const std::vector<std::shared_ptr<Transaction>>& transactions,
    QWidget* parent)
    : QDialog(parent), transactions_(transactions)
{
    setWindowTitle(tr("Review Transactions"));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    transactionWidget_ = new QWidget(this);
    mainLayout->addWidget(transactionWidget_);

    QHBoxLayout* navLayout = new QHBoxLayout();
    prevBtn_ = new QPushButton(tr("Zurück"), this);
    nextBtn_ = new QPushButton(tr("Weiter"), this);
    okBtn_ = new QPushButton(tr("OK"), this);

    navLayout->addWidget(prevBtn_);
    navLayout->addWidget(nextBtn_);
    navLayout->addStretch();
    navLayout->addWidget(okBtn_);
    mainLayout->addLayout(navLayout);

    connect(prevBtn_, &QPushButton::clicked, this, &TransactionReviewDialog::onPrev);
    connect(nextBtn_, &QPushButton::clicked, this, &TransactionReviewDialog::onNext);
    connect(okBtn_, &QPushButton::clicked, this, &QDialog::accept);

    showTransaction(currentIndex_);
}

void TransactionReviewDialog::showTransaction(int index) {
    delete transactionWidget_->layout();
    QVBoxLayout* layout = new QVBoxLayout(transactionWidget_);

    if (index < 0 || index >= static_cast<int>(transactions_.size()))
        return;

    auto tx = transactions_[index];
    if (!tx) {
        QLabel* errorLabel = new QLabel(tr("Fehler: Ungültige Transaktion!"), transactionWidget_);
        layout->addWidget(errorLabel);
        return;
    }

    QFormLayout* form = new QFormLayout();

    // Standardfelder mit QLineEdit
    QLineEdit* bookingDateEdit = new QLineEdit(QString::fromStdString(tx->bookingDate), transactionWidget_);
    form->addRow(tr("Booking Date"), bookingDateEdit);
    connect(bookingDateEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->bookingDate = text.toStdString();
    });

    QLineEdit* valutaDateEdit = new QLineEdit(QString::fromStdString(tx->valutaDate), transactionWidget_);
    form->addRow(tr("Valuta Date"), valutaDateEdit);
    connect(valutaDateEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->valutaDate = text.toStdString();
    });

    QLineEdit* amountEdit = new QLineEdit(QString::fromStdString(tx->amountText), transactionWidget_);
    form->addRow(tr("Amount"), amountEdit);
    connect(amountEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->amountText = text.toStdString();
    });

    // Details-Feld: mehrzeilig und größer
    QTextEdit* detailsEdit = new QTextEdit(QString::fromStdString(tx->details), transactionWidget_);
    detailsEdit->setMinimumHeight(120);
    form->addRow(tr("Details"), detailsEdit);
    connect(detailsEdit, &QTextEdit::textChanged, this, [tx, detailsEdit]() {
        tx->details = detailsEdit->toPlainText().toStdString();
    });

    layout->addLayout(form);

    prevBtn_->setEnabled(currentIndex_ > 0);
    nextBtn_->setEnabled(currentIndex_ < static_cast<int>(transactions_.size()) - 1);
}

void TransactionReviewDialog::onPrev() {
    if (currentIndex_ > 0) {
        --currentIndex_;
        showTransaction(currentIndex_);
    }
}

void TransactionReviewDialog::onNext() {
    if (currentIndex_ < static_cast<int>(transactions_.size()) - 1) {
        ++currentIndex_;
        showTransaction(currentIndex_);
    }
}