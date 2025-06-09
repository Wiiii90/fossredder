#pragma once
#include <QWidget>

class QListWidget;
class QPushButton;

class ManagePropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManagePropertiesWidget(QWidget* parent = nullptr);

private:
    QListWidget* propertiesList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};