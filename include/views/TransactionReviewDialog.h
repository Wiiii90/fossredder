#pragma once
#include <QDialog>
#include <vector>
#include <memory>
#include "models/Transaction.h"

class QLabel;
class QPushButton;

class TransactionReviewDialog : public QDialog {
    Q_OBJECT
public:
    TransactionReviewDialog(const std::vector<std::shared_ptr<Transaction>>& transactions, QWidget* parent = nullptr);
private:
    std::vector<std::shared_ptr<Transaction>> transactions_;
    int currentIndex_ = 0;

    QWidget* transactionWidget_ = nullptr;
    QPushButton* prevBtn_ = nullptr;
    QPushButton* nextBtn_ = nullptr;
    QPushButton* okBtn_ = nullptr;

    void showTransaction(int index);
private slots:
    void onPrev();
    void onNext();
};