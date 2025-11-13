#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTableWidget>
#include <memory>
#include <vector>
#include "models/layout/Transaction.h"
#include "controllers/StatementController.h"

class ManageStatementsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageStatementsWidget(std::shared_ptr<StatementController> pdfController, QWidget* parent = nullptr);

private slots:
    void onTransactionsExtracted(const std::vector<std::shared_ptr<Transaction>>& transactions);

private:
    std::shared_ptr<StatementController> pdfController_;

    QComboBox* propertyFilter_;
    QComboBox* actorFilter_;
    QLineEdit* searchEdit_;
    QListWidget* statementsList_;
    QListWidget* groupsList_;
    QTableWidget* transactionsTable_;
    QPushButton* importStatementBtn_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};