#pragma once
#include <QWidget>
#include <memory>

class QListWidget;
class QPushButton;
class ICategoryManager;

class ManageCategoriesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageCategoriesWidget(std::shared_ptr<ICategoryManager> manager, QWidget* parent = nullptr);

private:
    void refreshList();

    std::shared_ptr<ICategoryManager> manager_;
    QListWidget* categoriesList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};