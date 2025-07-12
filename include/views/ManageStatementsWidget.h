#pragma once
#include "controllers/PdfImportController.h"
#include <QWidget>
#include <memory>

class QComboBox;
class QLineEdit;
class QPushButton;
class QListWidget;
class QLabel;
class QTableWidget;

class ManageStatementsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageStatementsWidget(std::shared_ptr<PdfImportController> pdfController, QWidget* parent = nullptr);

private:
    std::shared_ptr<PdfImportController> pdfController_;
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