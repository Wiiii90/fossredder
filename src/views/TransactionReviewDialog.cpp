#include "views/TransactionReviewDialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
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
    // Lösche alten Inhalt
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

    auto addField = [&](const QString& label, std::string& value) {
        QLineEdit* edit = new QLineEdit(QString::fromStdString(value), transactionWidget_);
        form->addRow(label, edit);
        connect(edit, &QLineEdit::textChanged, this, [tx, &value](const QString& text) {
            value = text.toStdString();
            });
        };
    addField(tr("Booking Date"), tx->bookingDate);
    addField(tr("Valuta Date"), tx->valutaDate);
    addField(tr("Amount"), tx->amountText);
    addField(tr("Details"), tx->details);

    // Dynamisch extrahierte Felder
    for (auto it = tx->extractedFields.begin(); it != tx->extractedFields.end(); ++it) {
        QString key = QString::fromStdString(it->first);
        QLineEdit* edit = new QLineEdit(QString::fromStdString(it->second), transactionWidget_);
        form->addRow(key, edit);
        connect(edit, &QLineEdit::textChanged, this, [tx, key](const QString& text) {
            tx->extractedFields[key.toStdString()] = text.toStdString();
            });
    }

    // Buttons für Feld hinzufügen/entfernen
    QHBoxLayout* fieldButtons = new QHBoxLayout();
    QPushButton* addFieldBtn = new QPushButton(tr("Feld hinzufügen"), transactionWidget_);
    QPushButton* removeFieldBtn = new QPushButton(tr("Feld entfernen"), transactionWidget_);
    fieldButtons->addWidget(addFieldBtn);
    fieldButtons->addWidget(removeFieldBtn);
    form->addRow(fieldButtons);

    connect(addFieldBtn, &QPushButton::clicked, this, [form, tx, transactionWidget_ = transactionWidget_]() {
        bool ok;
        QString key = QInputDialog::getText(transactionWidget_, tr("Feld hinzufügen"), tr("Feldname:"), QLineEdit::Normal, "", &ok);
        if (ok && !key.isEmpty()) {
            QLineEdit* edit = new QLineEdit(transactionWidget_);
            form->addRow(key, edit);
            tx->extractedFields[key.toStdString()] = "";
            QObject::connect(edit, &QLineEdit::textChanged, transactionWidget_, [tx, key](const QString& text) {
                tx->extractedFields[key.toStdString()] = text.toStdString();
                });
        }
        });

    connect(removeFieldBtn, &QPushButton::clicked, this, [form, tx, transactionWidget_ = transactionWidget_]() {
        bool ok;
        QString key = QInputDialog::getText(transactionWidget_, tr("Feld entfernen"), tr("Feldname zum Entfernen:"), QLineEdit::Normal, "", &ok);
        if (ok && !key.isEmpty()) {
            tx->extractedFields.erase(key.toStdString());
            // UI-Entfernung: Findet das passende Widget und entfernt es
            for (int i = 0; i < form->rowCount(); ++i) {
                if (form->itemAt(i, QFormLayout::LabelRole)) {
                    QLabel* label = qobject_cast<QLabel*>(form->itemAt(i, QFormLayout::LabelRole)->widget());
                    if (label && label->text() == key) {
                        QWidget* field = form->itemAt(i, QFormLayout::FieldRole)->widget();
                        form->removeRow(i);
                        if (field) field->deleteLater();
                        if (label) label->deleteLater();
                        break;
                    }
                }
            }
        }
        });

    layout->addLayout(form);

    // Navigation Buttons aktivieren/deaktivieren
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