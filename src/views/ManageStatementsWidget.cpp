#include "views/ManageStatementsWidget.h"
#include "views/ImportDialog.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>

ManageStatementsWidget::ManageStatementsWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Top bar: Property filter, Actor filter, Search
    QHBoxLayout* topBarLayout = new QHBoxLayout();
    QLabel* propertyFilterLabel = new QLabel(tr("Property:"), this);
    propertyFilter_ = new QComboBox(this);
    propertyFilter_->addItem(tr("All Properties")); // Placeholder

    QLabel* actorFilterLabel = new QLabel(tr("Actor:"), this);
    actorFilter_ = new QComboBox(this);
    actorFilter_->addItem(tr("All Actors")); // Placeholder

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

    // Middle: Statements, Booking Groups, Transactions
    QHBoxLayout* middleLayout = new QHBoxLayout();

    // Statements list
    QVBoxLayout* statementsLayout = new QVBoxLayout();
    QLabel* statementsLabel = new QLabel(tr("Statements"), this);
    statementsList_ = new QListWidget(this);
    statementsList_->setSelectionMode(QAbstractItemView::SingleSelection);
    statementsLayout->addWidget(statementsLabel);
    statementsLayout->addWidget(statementsList_);

    // Booking Groups list
    QVBoxLayout* groupsLayout = new QVBoxLayout();
    QLabel* groupsLabel = new QLabel(tr("Booking Groups"), this);
    groupsList_ = new QListWidget(this);
    groupsList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    groupsLayout->addWidget(groupsLabel);
    groupsLayout->addWidget(groupsList_);

    // Transactions table (for details)
    QVBoxLayout* transactionsLayout = new QVBoxLayout();
    QLabel* transactionsLabel = new QLabel(tr("Transactions"), this);
    transactionsTable_ = new QTableWidget(this);
    transactionsTable_->setColumnCount(5); // Example: Date, Amount, Actor, Description, Category
    QStringList headers = { tr("Date"), tr("Amount"), tr("Actor"), tr("Description"), tr("Category") };
    transactionsTable_->setHorizontalHeaderLabels(headers);
    transactionsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    transactionsLayout->addWidget(transactionsLabel);
    transactionsLayout->addWidget(transactionsTable_);

    // Add all columns to middle layout
    middleLayout->addLayout(statementsLayout, 1);
    middleLayout->addLayout(groupsLayout, 1);
    middleLayout->addLayout(transactionsLayout, 2);

    // Bottom: Import left, CRUD right
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

    // Assemble all layouts
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(middleLayout, 1);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    // Import button opens ImportDialog
    connect(importStatementBtn_, &QPushButton::clicked, this, [this]() {
        ImportDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            // TODO: Start import process with dlg.selectedFile(), dlg.selectedType(), etc.
        }
        });
}