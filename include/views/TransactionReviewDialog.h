#pragma once
#include <QDialog>
#include <QVector>
#include "models/Transaction.h"

class QLabel;
class QPushButton;

class TransactionReviewDialog : public QDialog {
    Q_OBJECT
public:
    TransactionReviewDialog(const QVector<Transaction>& transactions, QWidget* parent = nullptr);

private:
    int currentIndex_;
    QVector<Transaction> transactions_;
    QLabel* detailsLabel_;
    QPushButton* nextBtn_;
    QPushButton* prevBtn_;
    QPushButton* acceptBtn_;
    QPushButton* ignoreBtn_;

    void updateView();
};