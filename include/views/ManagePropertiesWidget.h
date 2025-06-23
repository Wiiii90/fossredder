#pragma once
#include <QWidget>
#include <memory>

class QListWidget;
class QPushButton;
class IPropertyManager;

class ManagePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManagePropertiesWidget(std::shared_ptr<IPropertyManager> manager, QWidget* parent = nullptr);

private:
    void refreshList();

    std::shared_ptr<IPropertyManager> manager_;
    QListWidget* propertiesList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};