#pragma once
#include <QWidget>

class QListWidget;
class QPushButton;

class ManageActorsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageActorsWidget(QWidget* parent = nullptr);

private:
    QListWidget* actorsList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};