#include "views/TransactionReviewDialog.h"
#include <QFormLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "models/layout/Transaction.h"

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

static inline QString joinMetadata(const std::map<std::string,std::string>& md) {
    QStringList lines;
    for (const auto& kv : md) {
        QString k = QString::fromStdString(kv.first);
        QString v = QString::fromStdString(kv.second);
        lines << (k + ":" + v);
    }
    return lines.join('\n');
}

static inline std::map<std::string,std::string> parseMetadata(const QString& text) {
    std::map<std::string,std::string> md;
    auto lines = text.split('\n');
    for (const auto& l : lines) {
        QString line = l.trimmed();
        if (line.isEmpty()) continue;
        int idx = line.indexOf(':');
        if (idx <= 0) continue;
        QString k = line.left(idx).trimmed();
        QString v = line.mid(idx+1).trimmed();
        md.emplace(k.toStdString(), v.toStdString());
    }
    return md;
}

void TransactionReviewDialog::showTransaction(int index) {
    if (transactionWidget_->layout()) {
        // delete existing layout to recreate
        auto old = transactionWidget_->layout();
        delete old;
    }
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

    // Booking date
    QLineEdit* bookingDateEdit = new QLineEdit(QString::fromStdString(tx->bookingDate), transactionWidget_);
    form->addRow(tr("Booking Date"), bookingDateEdit);
    connect(bookingDateEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->bookingDate = text.toStdString();
    });

    // Value / Valuta date
    QLineEdit* valueDateEdit = new QLineEdit(QString::fromStdString(tx->valueDate), transactionWidget_);
    form->addRow(tr("Valuta Date"), valueDateEdit);
    connect(valueDateEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->valueDate = text.toStdString();
    });

    // Amount (signed)
    QLineEdit* amountEdit = new QLineEdit(QString::number(tx->amount), transactionWidget_);
    form->addRow(tr("Amount"), amountEdit);
    connect(amountEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        bool ok = false;
        double v = text.toDouble(&ok);
        if (ok) tx->amount = v;
    });

    // Actor (payee/payer)
    QLineEdit* actorEdit = new QLineEdit(QString::fromStdString(tx->actor), transactionWidget_);
    form->addRow(tr("Actor"), actorEdit);
    connect(actorEdit, &QLineEdit::textChanged, this, [tx](const QString& text) {
        tx->actor = text.toStdString();
    });

    // Metadata: show as key:value per line
    QTextEdit* metadataEdit = new QTextEdit(joinMetadata(tx->metadata), transactionWidget_);
    metadataEdit->setMinimumHeight(120);
    form->addRow(tr("Metadata (key:value per line)"), metadataEdit);
    connect(metadataEdit, &QTextEdit::textChanged, this, [tx, metadataEdit]() {
        tx->metadata = parseMetadata(metadataEdit->toPlainText());
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