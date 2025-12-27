#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QTimer>
#include <QObject>
#include <QLibraryInfo>
#include <QDir>
#include <QCoreApplication>
#include "ui/UiActions.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSRedder");
    resize(1200, 800);

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    QAction* newFile = fileMenu->addAction(tr("New..."));
    QAction* openFile = fileMenu->addAction(tr("Open..."));
    QAction* saveFile = fileMenu->addAction(tr("Save"));
    QAction* saveFileAs = fileMenu->addAction(tr("Save As..."));
    fileMenu->addSeparator();
    QAction* imp = fileMenu->addAction(tr("Import..."));
    QAction* exp = fileMenu->addAction(tr("Export..."));
    fileMenu->addSeparator();
    QAction* quit = fileMenu->addAction(tr("Quit"));

    QMenu* viewMenu = menuBar()->addMenu(tr("View"));
    QAction* toggleSidebar = viewMenu->addAction(tr("Toggle Sidebar"));

    QMenu* helpMenu = menuBar()->addMenu(tr("Help"));
    QAction* about = helpMenu->addAction(tr("About"));

    connect(newFile, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(openFile, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(saveFile, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(saveFileAs, &QAction::triggered, this, &MainWindow::onSaveFileAs);

    connect(imp, &QAction::triggered, this, &MainWindow::onImport);
    connect(exp, &QAction::triggered, this, &MainWindow::onExport);
    connect(quit, &QAction::triggered, this, &QWidget::close);
    connect(about, &QAction::triggered, this, &MainWindow::onAbout);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->setMinimumSize(0, 0);

    m_quickWidget->installEventFilter(this);

    if (m_quickWidget->rootContext()) {
        m_quickWidget->rootContext()->setContextProperty("useNativeMenu", true);
        m_quickWidget->rootContext()->setContextProperty("actionImport", imp);
        m_quickWidget->rootContext()->setContextProperty("actionExport", exp);
        m_quickWidget->rootContext()->setContextProperty("actionQuit", quit);
        m_quickWidget->rootContext()->setContextProperty("actionToggleSidebar", toggleSidebar);
        m_quickWidget->rootContext()->setContextProperty("actionAbout", about);
        m_quickWidget->rootContext()->setContextProperty("uiActions", new UiActions(this));

        m_quickWidget->rootContext()->setContextProperty("statusText", QStringLiteral("Ready"));
        m_quickWidget->rootContext()->setContextProperty("statusItems", 3);

#ifdef _DEBUG
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", true);
#else
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", false);
#endif
    }

    setCentralWidget(m_quickWidget);

    Q_INIT_RESOURCE(qml);

    // Register standard Qt QML import path if available
    const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
        m_quickWidget->engine()->addImportPath(qtImports);
    }

    // Add application-local qml path
    const QString appQmlDir = QCoreApplication::applicationDirPath() + "/qml";
    if (QDir(appQmlDir).exists()) {
        m_quickWidget->engine()->addImportPath(appQmlDir);
    }

    // Ensure application-local imageformats folder is considered
    const QString imageFormatsDir = QCoreApplication::applicationDirPath() + "/imageformats";
    if (QDir(imageFormatsDir).exists()) {
        QCoreApplication::addLibraryPath(imageFormatsDir);
    }

    // Always ensure the embedded qrc import path is available
    m_quickWidget->engine()->addImportPath("qrc:/qml");
    m_quickWidget->setSource(QUrl("qrc:/qml/Main.qml"));

    // Ensure QML root gets initial size so anchored loaders layout correctly
    if (m_quickWidget->rootObject()) {
        QObject* root = m_quickWidget->rootObject();
        root->setProperty("width", m_quickWidget->width());
        root->setProperty("height", m_quickWidget->height());
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == m_quickWidget && ev->type() == QEvent::Resize) {
        if (m_quickWidget->rootObject()) {
            QQuickItem* root = m_quickWidget->rootObject();
            int w = m_quickWidget->width();
            int h = m_quickWidget->height();
            root->setProperty("width", w);
            root->setProperty("height", h);
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onImport()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Import File"));
    if (!file.isEmpty()) {
        if (m_quickWidget->rootContext()) m_quickWidget->rootContext()->setContextProperty("statusText", QString("Selected: %1").arg(file));
        // Emit signal instead of showing modal QMessageBox so controllers can handle import
        emit importRequested(file);
    }
}

void MainWindow::onExport()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Export File"));
    if (!file.isEmpty()) {
        if (m_quickWidget->rootContext()) m_quickWidget->rootContext()->setContextProperty("statusText", QString("Export path: %1").arg(file));
        QMessageBox::information(this, tr("Export"), tr("Export path: %1").arg(file));
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About FOSSRedder"), tr("FOSSRedder - demo"));
}

void MainWindow::onNewFile() {
    QString file = QFileDialog::getSaveFileName(this, tr("New File"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit newFileRequested(file);
}

void MainWindow::onOpenFile() {
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit openFileRequested(file);
}

void MainWindow::onSaveFile() {
    emit saveFileRequested();
}

void MainWindow::onSaveFileAs() {
    QString file = QFileDialog::getSaveFileName(this, tr("Save File As"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit saveFileAsRequested(file);
}
