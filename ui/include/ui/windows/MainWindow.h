#pragma once
#include <QMainWindow>
#include <memory>

class QStackedWidget;
class QAction;

class StatementController;
class IActorRepository;
class IPropertyRepository;
class IStatementRepository;
class IConfigRepository;
class ITransactionRepository;
class IBookingGroupRepository;

namespace ui {

class StatementTreeWidget;
class QTMainController;

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

private:
    void setup_ui();
    void setup_menus();
    void setup_toolbar();
    void setup_central_stack();

    StatementTreeWidget* m_statement_tree = nullptr;
    QStackedWidget* m_stack = nullptr;
    QTMainController* m_controller = nullptr;
    QAction* m_toggle_left_tree = nullptr;
    QAction* m_toggle_right_dock = nullptr;
};

}
