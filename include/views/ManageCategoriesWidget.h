#pragma once
#include <QWidget>

class QListWidget;
class QPushButton;

class ManageCategoriesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageCategoriesWidget(QWidget* parent = nullptr);

private:
    QListWidget* categoriesList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};