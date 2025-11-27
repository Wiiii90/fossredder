#pragma once
#include <QMainWindow>
#include <memory>
#include <QList>

class QStackedWidget;
class QAction;
class QWidget;

class StatementController;
class IActorRepository;
class IPropertyRepository;
class IStatementRepository;
class IConfigRepository;
class ITransactionRepository;
class IBookingGroupRepository;

namespace ui {

class TreeWidget;
class QTMainController;
class BaseView;
class ActionRegistry;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<StatementController> statementController = nullptr,
                        std::shared_ptr<IActorRepository> actorRepo = nullptr,
                        std::shared_ptr<IPropertyRepository> propertyRepo = nullptr,
                        std::shared_ptr<IStatementRepository> statementRepo = nullptr,
                        std::shared_ptr<IConfigRepository> configRepo = nullptr,
                        std::shared_ptr<ITransactionRepository> transactionRepo = nullptr,
                        std::shared_ptr<IBookingGroupRepository> bookingGroupRepo = nullptr,
                        QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onViewActionsChanged(const QStringList& labels);

private:
    TreeWidget* m_tree_widget = nullptr;
    QStackedWidget* m_stack = nullptr;
    QTMainController* m_controller = nullptr;
    QAction* m_toggle_left_tree = nullptr;
    QAction* m_toggle_right_dock = nullptr;
    QWidget* m_view_action_area = nullptr;

    ActionRegistry* m_action_registry = nullptr;

    QList<BaseView*> m_views;
};

}
