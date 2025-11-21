#include "ui/windows/MainWindow.h"
#include "ui/widgets/StatementTreeWidget.h"
#include "ui/controllers/QTMainController.h"
#include "ui/views/StatementsView.h"
#include "ui/views/TransactionView.h"
#include "ui/views/BookingGroupView.h"
#include "ui/views/YearlyStatementView.h"

#include <QStackedWidget>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QSettings>

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
    m_controller = new QTMainController(this);
    setup_ui();
    setup_menus();
    setup_toolbar();
    setup_central_stack();

    // connect tree selection to controller
    connect(m_statement_tree, &StatementTreeWidget::selectionContextChanged,
            m_controller, &QTMainController::onTreeSelectionChanged);

    // controller updates context
    connect(m_controller, &QTMainController::contextChanged, this, [this](ContextLevel level, const QList<QString>& ids){
        int index = 0;
        switch (level) {
            case ContextLevel::Root: index = 0; break;
            case ContextLevel::Statement: index = 1; break;
            case ContextLevel::BookingGroup: index = 2; break;
            case ContextLevel::Transaction: index = 3; break;
        }
        if (m_stack) m_stack->setCurrentIndex(index);

        // update view context
        QWidget* w = m_stack->currentWidget();
        if (auto v = qobject_cast<StatementsView*>(w)) v->setContext(ids);
        if (auto v = qobject_cast<BookingGroupView*>(w)) v->setContext(ids);
        if (auto v = qobject_cast<TransactionView*>(w)) v->setContext(ids);
        if (auto v = qobject_cast<YearlyStatementView*>(w)) v->setContext(ids);

        statusBar()->showMessage(QString("Context: %1, items: %2").arg(static_cast<int>(level)).arg(ids.size()));
    });

    QSettings settings;
    restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
    restoreState(settings.value("mainwindow/state").toByteArray());
}

MainWindow::~MainWindow() {
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/state", saveState());
}

void MainWindow::setup_ui() {
    QSplitter* splitter = new QSplitter(this);

    m_statement_tree = new StatementTreeWidget(splitter);

    m_stack = new QStackedWidget(splitter);

    splitter->addWidget(m_statement_tree);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    QDockWidget* rightDock = new QDockWidget(tr("Analytics"), this);
    rightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    rightDock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);
}

void MainWindow::setup_menus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Import Statement"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), this, &MainWindow::close);

    QMenu* viewMenu = menuBar()->addMenu(tr("View"));
    m_toggle_left_tree = viewMenu->addAction(tr("Show Left Tree"));
    m_toggle_left_tree->setCheckable(true);
    m_toggle_left_tree->setChecked(true);
    connect(m_toggle_left_tree, &QAction::toggled, m_statement_tree, &QWidget::setVisible);

    m_toggle_right_dock = viewMenu->addAction(tr("Show Right Sidebar"));
    m_toggle_right_dock->setCheckable(true);
    m_toggle_right_dock->setChecked(false);
    connect(m_toggle_right_dock, &QAction::toggled, this, [this](bool visible){
        for (QDockWidget* dock : findChildren<QDockWidget*>()) dock->setVisible(visible);
    });
}

void MainWindow::setup_toolbar() {
    QToolBar* tb = addToolBar(tr("Main"));
    tb->addAction(tr("Import"));
    tb->addAction(tr("Zoom In"));
    tb->addAction(tr("Zoom Out"));

    // context area placeholder
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    QLabel* ctxLabel = new QLabel(tr("Context Actions"), this);
    tb->addWidget(ctxLabel);
}

void MainWindow::setup_central_stack() {
    auto statements = new StatementsView(this);
    auto booking = new BookingGroupView(this);
    auto transactions = new TransactionView(this);
    auto yearly = new YearlyStatementView(this);

    m_stack->addWidget(statements);
    m_stack->addWidget(booking);
    m_stack->addWidget(transactions);
    m_stack->addWidget(yearly);
}

}
