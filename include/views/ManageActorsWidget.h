#pragma once
#include <QWidget>
#include <memory>

class QListWidget;
class QPushButton;
class IActorManager;

class ManageActorsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManageActorsWidget(std::shared_ptr<IActorManager> manager, QWidget* parent = nullptr);

private:
    void refreshList();

    std::shared_ptr<IActorManager> manager_;
    QListWidget* actorsList_;
    QPushButton* addBtn_;
    QPushButton* editBtn_;
    QPushButton* deleteBtn_;
};