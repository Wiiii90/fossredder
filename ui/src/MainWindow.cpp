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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSRedder");
    resize(1200, 800);

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    QAction* imp = fileMenu->addAction(tr("Import..."));
    QAction* exp = fileMenu->addAction(tr("Export..."));
    fileMenu->addSeparator();
    QAction* quit = fileMenu->addAction(tr("Quit"));

    QMenu* viewMenu = menuBar()->addMenu(tr("View"));
    QAction* toggleSidebar = viewMenu->addAction(tr("Toggle Sidebar"));

    QMenu* helpMenu = menuBar()->addMenu(tr("Help"));
    QAction* about = helpMenu->addAction(tr("About"));

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

        m_quickWidget->rootContext()->setContextProperty("statusText", QStringLiteral("Ready"));
        m_quickWidget->rootContext()->setContextProperty("statusItems", 3);
    }

    setCentralWidget(m_quickWidget);

    Q_INIT_RESOURCE(qml);

    // Register standard Qt QML import path if available (keeps Controls/Layouts available)
    const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
        m_quickWidget->engine()->addImportPath(qtImports);
    }

    // Add vcpkg / app-local qml paths if present (no debug output)
    QString vcpkgInstalled = QString::fromUtf8(qgetenv("VCPKG_INSTALLED_DIR"));
    QString triplet = QString::fromUtf8(qgetenv("VCPKG_TARGET_TRIPLET"));
    QStringList candidates;
    if (!vcpkgInstalled.isEmpty() && !triplet.isEmpty()) {
        candidates << QDir::cleanPath(vcpkgInstalled + "/" + triplet + "/share/qt6/qml")
                   << QDir::cleanPath(vcpkgInstalled + "/" + triplet + "/share/qt/qml")
                   << QDir::cleanPath(vcpkgInstalled + "/" + triplet + "/plugins/qml")
                   << QDir::cleanPath(vcpkgInstalled + "/" + triplet + "/qml")
                   << QDir::cleanPath(vcpkgInstalled + "/share/qt6/qml")
                   << QDir::cleanPath(vcpkgInstalled + "/qml");
    }

    const QString appQmlDir = QCoreApplication::applicationDirPath() + "/qml";
    candidates << appQmlDir;

    for (const QString &p : candidates) {
        if (!p.isEmpty() && QDir(p).exists()) {
            m_quickWidget->engine()->addImportPath(p);
        }
    }

    // Always ensure the embedded qrc import path is available
    m_quickWidget->engine()->addImportPath("qrc:/qml");
    m_quickWidget->setSource(QUrl("qrc:/qml/Main.qml"));

    // Ensure QML root gets initial size so anchored loaders layout correctly
    if (m_quickWidget->rootObject()) {
        QObject* root = m_quickWidget->rootObject();
        root->setProperty("width", m_quickWidget->width());
        root->setProperty("height", m_quickWidget->height());
        qDebug() << "Initialized QML root size to:" << m_quickWidget->width() << m_quickWidget->height();
    }
    QTimer::singleShot(0, this, [this]() {
        if (m_quickWidget && m_quickWidget->rootObject()) {
            QObject* root = m_quickWidget->rootObject();
            root->setProperty("width", m_quickWidget->width());
            root->setProperty("height", m_quickWidget->height());
            qDebug() << "Deferred QML root size update to:" << m_quickWidget->width() << m_quickWidget->height();
        }
    });
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
            qDebug() << "Updated QML root size to:" << w << h;
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
        QMessageBox::information(this, tr("Import"), tr("Selected: %1").arg(file));
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
