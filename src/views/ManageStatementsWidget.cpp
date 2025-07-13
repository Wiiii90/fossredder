#include "views/ManageStatementsWidget.h"
#include "views/TransactionReviewDialog.h"
#include "models/PdfExtractedData.h"
#include "models/BookingGroup.h"
#include "controllers/PdfImportController.h"
#include <QApplication>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageStatementsWidget::ManageStatementsWidget(
    std::shared_ptr<PdfImportController> pdfController,
    QWidget* parent)
    : QWidget(parent), pdfController_(std::move(pdfController))
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* topBarLayout = new QHBoxLayout();
    QLabel* propertyFilterLabel = new QLabel(tr("Property:"), this);
    propertyFilter_ = new QComboBox(this);
    propertyFilter_->addItem(tr("All Properties"));

    QLabel* actorFilterLabel = new QLabel(tr("Actor:"), this);
    actorFilter_ = new QComboBox(this);
    actorFilter_->addItem(tr("All Actors"));

    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText(tr("Search..."));
    searchEdit_->setMinimumWidth(200);

    topBarLayout->addWidget(propertyFilterLabel);
    topBarLayout->addWidget(propertyFilter_);
    topBarLayout->addSpacing(10);
    topBarLayout->addWidget(actorFilterLabel);
    topBarLayout->addWidget(actorFilter_);
    topBarLayout->addStretch();
    topBarLayout->addWidget(searchEdit_);

    QHBoxLayout* middleLayout = new QHBoxLayout();

    QVBoxLayout* statementsLayout = new QVBoxLayout();
    QLabel* statementsLabel = new QLabel(tr("Statements"), this);
    statementsList_ = new QListWidget(this);
    statementsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    statementsLayout->addWidget(statementsLabel);
    statementsLayout->addWidget(statementsList_);

    QVBoxLayout* groupsLayout = new QVBoxLayout();
    QLabel* groupsLabel = new QLabel(tr("Booking Groups"), this);
    groupsList_ = new QListWidget(this);
    groupsList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    groupsLayout->addWidget(groupsLabel);
    groupsLayout->addWidget(groupsList_);

    QVBoxLayout* transactionsLayout = new QVBoxLayout();
    QLabel* transactionsLabel = new QLabel(tr("Transactions"), this);
    transactionsTable_ = new QTableWidget(this);
    transactionsTable_->setColumnCount(5);
    QStringList headers = { tr("Date"), tr("Amount"), tr("Actor"), tr("Description"), tr("Category") };
    transactionsTable_->setHorizontalHeaderLabels(headers);
    transactionsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    transactionsLayout->addWidget(transactionsLabel);
    transactionsLayout->addWidget(transactionsTable_);

    middleLayout->addLayout(statementsLayout, 1);
    middleLayout->addLayout(groupsLayout, 1);
    middleLayout->addLayout(transactionsLayout, 2);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    importStatementBtn_ = new QPushButton(tr("Import Statement..."), this);
    bottomLayout->addWidget(importStatementBtn_, 0, Qt::AlignLeft);
    bottomLayout->addStretch();
    addBtn_ = new QPushButton(tr("Add"), this);
    editBtn_ = new QPushButton(tr("Edit"), this);
    deleteBtn_ = new QPushButton(tr("Delete"), this);
    bottomLayout->addWidget(addBtn_, 0, Qt::AlignRight);
    bottomLayout->addWidget(editBtn_, 0, Qt::AlignRight);
    bottomLayout->addWidget(deleteBtn_, 0, Qt::AlignRight);

    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(middleLayout, 1);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    connect(importStatementBtn_, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select PDF"), QString(), tr("PDF Files (*.pdf)"));
        if (filePath.isEmpty())
            return;

        QProgressDialog progress(tr("Importing PDF..."), tr("Cancel"), 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        QApplication::processEvents();

        try {
            pdfController_->extractData(filePath.toStdString());
            progress.close();
        } catch (const std::exception& ex) {
            progress.close();
            QMessageBox::critical(this, tr("Import Error"), QString::fromStdString(ex.what()));
        }
    });

    connect(pdfController_.get(), &PdfImportController::transactionsExtracted,
            this, &ManageStatementsWidget::onTransactionsExtracted);
}

void ManageStatementsWidget::onTransactionsExtracted(const std::vector<std::shared_ptr<Transaction>>& transactions) {
    qDebug() << "onTransactionsExtracted called. Anzahl Transaktionen:" << transactions.size();
    for (size_t i = 0; i < transactions.size(); ++i) {
        if (!transactions[i]) {
            qDebug() << "Transaktion" << i << "ist nullptr!";
        } else {
            qDebug() << "Transaktion" << i << "BookingDate:" << QString::fromStdString(transactions[i]->bookingDate)
                     << "AmountText:" << QString::fromStdString(transactions[i]->amountText);
        }
    }
    TransactionReviewDialog dlg(transactions, this);
    dlg.exec();
}