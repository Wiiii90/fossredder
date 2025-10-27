#include "views/MainWindow.h"
#include "views/DashboardWidget.h"
#include "views/ManageStatementsWidget.h"
#include "views/ManageActorsWidget.h"
#include "views/ManageCategoriesWidget.h"
#include "views/ManagePropertiesWidget.h"
#include "views/ManageAnnualCostWidget.h"
#include "managers/ActorManager.h"
#include "managers/CategoryManager.h"
#include "managers/PropertyManager.h"
#include "controllers/PdfImportController.h"
#include "ocr/TesseractOcrEngine.h"
#include "poppler/PopplerPdfRenderer.h"
#include "onnx/OnnxTextCleaner.h"
#include "onnx/OnnxConfigFactory.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSredder");
    resize(1024, 768);

    // Menu bar
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* openAction = fileMenu->addAction(tr("Open Save File..."));
    QAction* saveAction = fileMenu->addAction(tr("Save"));
    QAction* saveAsAction = fileMenu->addAction(tr("Save As..."));
    fileMenu->addSeparator();
    QAction* importAction = fileMenu->addAction(tr("Import..."));
    QAction* exportAction = fileMenu->addAction(tr("Export..."));
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(tr("Exit"));

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    QMenu* settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction* preferencesAction = settingsMenu->addAction(tr("Preferences..."));
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* aboutAction = helpMenu->addAction(tr("About FOSSredder"));

    // Status bar
    statusBar()->showMessage(tr("Ready"));

    // Central widget layout
    QWidget* central = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);

    // Sidebar (vertical)
    QWidget* sidebar = new QWidget(central);
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setAlignment(Qt::AlignTop);

    QPushButton* dashboardBtn = new QPushButton(tr("Dashboard"), sidebar);
    QPushButton* manageStatementsBtn = new QPushButton(tr("Manage Statements"), sidebar);
    QPushButton* manageActorsBtn = new QPushButton(tr("Manage Actors"), sidebar);
    QPushButton* manageCategoriesBtn = new QPushButton(tr("Manage Categories"), sidebar);
    QPushButton* managePropertiesBtn = new QPushButton(tr("Manage Properties"), sidebar);
    QPushButton* manageAnnualCostBtn = new QPushButton(tr("Manage Annual Cost Statements"), sidebar);

    sidebarLayout->addWidget(dashboardBtn);
    sidebarLayout->addWidget(manageStatementsBtn);
    sidebarLayout->addWidget(manageActorsBtn);
    sidebarLayout->addWidget(manageCategoriesBtn);
    sidebarLayout->addWidget(managePropertiesBtn);
    sidebarLayout->addWidget(manageAnnualCostBtn);
    sidebar->setLayout(sidebarLayout);

    // Stacked widget for main content
    m_stack = new QStackedWidget(central);

    // Dashboard view
    DashboardWidget* dashboardWidget = new DashboardWidget(this);

    // Manager creation
    auto actorManager = std::make_shared<ActorManager>();
    auto categoryManager = std::make_shared<CategoryManager>();
    auto propertyManager = std::make_shared<PropertyManager>();

    // PDF Controller creation
    auto ocrEngine = std::make_shared<TesseractOcrEngine>();
    auto pdfRenderer = std::make_shared<PopplerPdfRenderer>();

    // Modell/Tokenizer-Auswahl jetzt über .env
    std::shared_ptr<ITextCleaner> textCleaner;
    {
        auto modelPaths = make_model_paths_from_env();
        if (!modelPaths.encoder.empty() && !modelPaths.decoder.empty() && !modelPaths.init_decoder.empty()) {
            textCleaner = std::make_shared<OnnxTextCleaner>(modelPaths);
        } else {
            textCleaner = nullptr; // Kein Cleaner, falls Pfade fehlen
        }
    }

    auto pdfController = std::make_shared<PdfImportController>(ocrEngine, pdfRenderer, textCleaner);

    // Manage Statements view
    ManageStatementsWidget* manageStatementsWidget = new ManageStatementsWidget(pdfController, this);

    // Manage Actors view
    ManageActorsWidget* manageActorsWidget = new ManageActorsWidget(actorManager, this);

    // Manage Categories view
    ManageCategoriesWidget* manageCategoriesWidget = new ManageCategoriesWidget(categoryManager, this);

    // Manage Properties view
    ManagePropertiesWidget* managePropertiesWidget = new ManagePropertiesWidget(propertyManager, this);

    // Manage Annual Cost Statements view
    ManageAnnualCostWidget* manageAnnualCostWidget = new ManageAnnualCostWidget(this);

    // Add views to the stack
    m_stack->addWidget(dashboardWidget);           // Index 0
    m_stack->addWidget(manageStatementsWidget);    // Index 1
    m_stack->addWidget(manageActorsWidget);        // Index 2
    m_stack->addWidget(manageCategoriesWidget);    // Index 3
    m_stack->addWidget(managePropertiesWidget);    // Index 4
    m_stack->addWidget(manageAnnualCostWidget);    // Index 5

    // Add sidebar and stack to main layout
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(m_stack, 1);

    setCentralWidget(central);

    // Show dashboard by default
    m_stack->setCurrentIndex(0);

    // Sidebar navigation
    connect(dashboardBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        statusBar()->showMessage(tr("Dashboard selected"));
        });
    connect(manageStatementsBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(1);
        statusBar()->showMessage(tr("Manage Statements selected"));
        });
    connect(manageActorsBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(2);
        statusBar()->showMessage(tr("Manage Actors selected"));
        });
    connect(manageCategoriesBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(3);
        statusBar()->showMessage(tr("Manage Categories selected"));
        });
    connect(managePropertiesBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(4);
        statusBar()->showMessage(tr("Manage Properties selected"));
        });
    connect(manageAnnualCostBtn, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(5);
        statusBar()->showMessage(tr("Manage Annual Cost Statements selected"));
        });

    // Connect actions (File menu)
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About FOSSredder"), tr("FOSSredder\nOpen Source PDF Extraction and OCR"));
        });
    // TODO: Connect open/save/import/export/preferences actions as needed
}

MainWindow::~MainWindow() = default;