#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QListWidget;
class QTableWidget;

class ManageStatementsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageStatementsWidget(QWidget* parent = nullptr);

private:
    QComboBox* propertyFilter_;
    QComboBox* actorFilter_;
    QLineEdit* searchEdit_;
    QPushButton* importStatementBtn_;
    QListWidget* statementsList_;
    QListWidget* groupsList_;
    QTableWidget* transactionsTable_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};