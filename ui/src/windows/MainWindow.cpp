#include "ui/windows/MainWindow.h"
#include "ui/windows/Workspace.h"
#include "ui/widgets/TreeWidget.h"
#include "ui/controllers/QTMainWindowController.h"
#include "ui/controllers/QTMainController.h"
#include "ui/views/StatementsView.h"
#include "ui/views/TransactionView.h"
#include "ui/views/BookingGroupView.h"
#include "ui/views/AnnualView.h"
#include "ui/actions/ActionRegistry.h"
#include "ui/widgets/ToolBarWidget.h"
#include "ui/views/ViewFactory.h"
#include "ui/widgets/BackgroundWidget.h"

#include <QStackedWidget>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QPushButton>
#include <QFrame>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QColor>

namespace ui {

MainWindow::MainWindow(std::shared_ptr<StatementController> /*statementController*/,
                       std::shared_ptr<IActorRepository> /*actorRepo*/,
                       std::shared_ptr<IPropertyRepository> /*propertyRepo*/,
                       std::shared_ptr<IStatementRepository> /*statementRepo*/,
                       std::shared_ptr<IConfigRepository> /*configRepo*/,
                       std::shared_ptr<ITransactionRepository> /*transactionRepo*/,
                       std::shared_ptr<IBookingGroupRepository> /*bookingGroupRepo*/,
                       QWidget* parent)
    : QMainWindow(parent)
{
    // MainWindow now defers to QTMainWindowController for wiring and view creation
    m_controller = nullptr;
    m_action_registry = nullptr;

    // create UI shell
    Workspace* workspace = new Workspace(this);
    setCentralWidget(workspace);

    // create controller which will create domain controller and views
    QTMainWindowController* ctrl = new QTMainWindowController(this, this);
    // adopt references from controller
    m_controller = ctrl->qtController();
    m_action_registry = ctrl->actionRegistry();
    m_views = ctrl->views();

    // let controller build menus and toolbar now that it has action registry and views
    ctrl->setupMenus(this);
    ctrl->setupToolbar(this);

    // connect controller's domain controller
    if (m_controller) {
        connect(workspace->treeWidget(), &TreeWidget::selectionContextChanged,
                m_controller, &QTMainController::onTreeSelectionChanged);

        connect(m_controller, &QTMainController::contextChanged, this, [this, workspace](ContextLevel level, const QList<QString>& ids){
            int stackIndex = 0;
            switch (level) {
                case ContextLevel::Root:
                    stackIndex = 0;
                    break;
                case ContextLevel::Annual:
                    stackIndex = 0;
                    break;
                case ContextLevel::Statement:
                    stackIndex = 1;
                    break;
                case ContextLevel::BookingGroup:
                    stackIndex = 2;
                    break;
                case ContextLevel::Transaction:
                    stackIndex = 3;
                    break;
                default:
                    stackIndex = 0;
                    break;
            }

            workspace->setCurrentIndex(stackIndex);

            QWidget* w = nullptr;
            // find current widget from workspace stacked widget
            QWidget* central = workspace->centralWidget();
            if (central) w = central->findChild<QStackedWidget*>()->currentWidget();
            if (auto v = qobject_cast<AnnualView*>(w)) v->setContext(ids, level);
            if (auto v = qobject_cast<StatementsView*>(w)) v->setContext(ids, level);
            if (auto v = qobject_cast<BookingGroupView*>(w)) v->setContext(ids, level);
            if (auto v = qobject_cast<TransactionView*>(w)) v->setContext(ids, level);

            statusBar()->showMessage(QString("Context: %1").arg(static_cast<int>(level)));
        });
    }

    workspace->setViews(m_views);

    QSettings settings;
    restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
    restoreState(settings.value("mainwindow/state").toByteArray());

    resize(1200, 800);
    showMaximized();
}

void MainWindow::onViewActionsChanged(const QStringList& labels) {
    Q_UNUSED(labels);
    // Currently no-op; placeholder to satisfy moc linkage and allow future wiring
}

MainWindow::~MainWindow() {
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/state", saveState());
}

} // namespace ui
