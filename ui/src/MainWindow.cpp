#include "MainWindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlImageProviderBase>
#include <QQuickItem>
#include <QQuickWidget>
#include <QSizePolicy>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QRegularExpression>

#include "ui/actions/UiActions.h"
#include "ui/menus/NativeMenu.h"
#include "ui/state/UiDataSession.h"
#include "ui/state/UiNavigation.h"
#include "ui/controllers/UiFileSystem.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSRedder");
    resize(1200, 800);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->setMinimumSize(0, 0);

    // Required so QML `DropArea` receives native drag&drop events from the OS.
    m_quickWidget->setAcceptDrops(true);

    m_quickWidget->installEventFilter(this);

    // Ensure UiNavigation enums are visible in QML as UiNavigation.Actors/...
    if (m_quickWidget->engine()) {
        qmlRegisterUncreatableType<UiNavigation>("FossRedder", 1, 0, "UiNavigation",
                                                "UiNavigation is exposed via context property 'uiNav'");
    }

    auto actions = new UiActions(this);
    actions_ = actions;
    auto nav = new UiNavigation(this);
    dataSession_ = new UiDataSession(this);
    auto fileSys = new UiFileSystem(this);

    if (m_quickWidget->rootContext()) {
        m_quickWidget->rootContext()->setContextProperty("uiActions", actions);
        m_quickWidget->rootContext()->setContextProperty("uiNav", nav);
        m_quickWidget->rootContext()->setContextProperty("uiData", dataSession_);
        m_quickWidget->rootContext()->setContextProperty("uiFileSystem", fileSys);

        (void)new NativeMenu(this, actions, m_quickWidget->rootContext(), this);

#ifdef _DEBUG
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", true);
#else
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", false);
#endif

        m_quickWidget->rootContext()->setContextProperty("statusText", QStringLiteral("Ready"));
        m_quickWidget->rootContext()->setContextProperty("statusItems", 3);
    }

    connect(actions->newFileAction(), &QAction::triggered, this, &MainWindow::onNewFile);
    connect(actions->openFileAction(), &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(actions->saveFileAction(), &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(actions->saveFileAsAction(), &QAction::triggered, this, &MainWindow::onSaveFileAs);

    connect(actions->importAction(), &QAction::triggered, this, &MainWindow::onImport);
    connect(actions->exportAction(), &QAction::triggered, this, &MainWindow::onExport);
    connect(actions->quitAction(), &QAction::triggered, this, &QWidget::close);
    connect(actions->aboutAction(), &QAction::triggered, this, &MainWindow::onAbout);

    connect(actions, &UiActions::importBrowseRequested, this, [this, actions](const QString& filter) {
        const QStringList files = QFileDialog::getOpenFileNames(this, tr("Select PDF"), QString(), filter);
        if (!files.isEmpty()) {
            emit actions->importFilesSelected(files);
            // keep single-file signal for backwards compatibility
            if (files.size() == 1) emit actions->importFileSelected(files.first());
        }
    });

    connect(actions, &UiActions::exportBrowseRequested, this, [this, actions](const QString& filter) {
        const QString file = QFileDialog::getSaveFileName(this, tr("Export File"), QString(), filter);
        if (!file.isEmpty()) {
            emit actions->exportFileSelected(file);
        }
    });

    setCentralWidget(m_quickWidget);

    Q_INIT_RESOURCE(qml);

    const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
        m_quickWidget->engine()->addImportPath(qtImports);
    }

    const QString appQmlDir = QCoreApplication::applicationDirPath() + "/qml";
    if (QDir(appQmlDir).exists()) {
        m_quickWidget->engine()->addImportPath(appQmlDir);
    }

    const QString imageFormatsDir = QCoreApplication::applicationDirPath() + "/imageformats";
    if (QDir(imageFormatsDir).exists()) {
        QCoreApplication::addLibraryPath(imageFormatsDir);
    }

    m_quickWidget->engine()->addImportPath("qrc:/qml");
}

static QStringList droppedLocalFiles(const QMimeData* md)
{
    QStringList out;
    if (!md) return out;
    if (!md->hasUrls()) return out;
    const auto urls = md->urls();
    for (const auto& u : urls) {
        const QString lf = u.toLocalFile();
        if (lf.isEmpty()) continue;
        out.push_back(lf);
    }
    return out;
}

MainWindow::~MainWindow() = default;

void MainWindow::setQmlContextProperty(const QString& name, QObject* value)
{
    if (!m_quickWidget) return;
    if (!m_quickWidget->rootContext()) return;
    m_quickWidget->rootContext()->setContextProperty(name, value);
}

void MainWindow::addImageProvider(const QString& id, QQmlImageProviderBase* provider)
{
    if (!m_quickWidget) return;
    if (!m_quickWidget->engine()) return;
    m_quickWidget->engine()->addImageProvider(id, provider);
}

void MainWindow::loadQml(const QUrl& source)
{
    if (!m_quickWidget) return;
    if (m_quickWidget->source() == source) return;
    m_quickWidget->setSource(source);

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
            root->setProperty("width", m_quickWidget->width());
            root->setProperty("height", m_quickWidget->height());
        }
    }

    // QQuickWidget in widget-embedding mode is often unreliable with QML DropArea.
    // Handle drag&drop on the widget layer and forward file paths into QML via UiActions.
    if (obj == m_quickWidget) {
        switch (ev->type()) {
        case QEvent::DragEnter: {
            auto* e = static_cast<QDragEnterEvent*>(ev);
            const auto files = droppedLocalFiles(e->mimeData());
            if (!files.isEmpty()) e->acceptProposedAction();
            else e->ignore();
            return true;
        }
        case QEvent::DragMove: {
            auto* e = static_cast<QDragMoveEvent*>(ev);
            const auto files = droppedLocalFiles(e->mimeData());
            if (!files.isEmpty()) e->acceptProposedAction();
            else e->ignore();
            return true;
        }
        case QEvent::Drop: {
            auto* e = static_cast<QDropEvent*>(ev);
            const auto files = droppedLocalFiles(e->mimeData());
            if (files.isEmpty()) {
                e->ignore();
                return true;
            }

            if (actions_) {
                emit actions_->importFilesDropped(files);
                if (files.size() == 1) emit actions_->importFileDropped(files.first());
            }
            e->acceptProposedAction();
            return true;
        }
        default:
            break;
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    emit saveFileRequested();
    QMainWindow::closeEvent(event);
}

void MainWindow::onImport()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Import File"));
    if (!file.isEmpty()) {
        if (m_quickWidget && m_quickWidget->rootContext()) {
            m_quickWidget->rootContext()->setContextProperty("statusText", QString("Selected: %1").arg(file));
        }
        emit importRequested(file);
    }
}

void MainWindow::onExport()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Export File"));
    if (!file.isEmpty()) {
        if (m_quickWidget && m_quickWidget->rootContext()) {
            m_quickWidget->rootContext()->setContextProperty("statusText", QString("Export path: %1").arg(file));
        }
        QMessageBox::information(this, tr("Export"), tr("Export path: %1").arg(file));
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About FOSSRedder"), tr("FOSSRedder - demo"));
}

void MainWindow::onNewFile()
{
    QString file = QFileDialog::getSaveFileName(this, tr("New File"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit newFileRequested(file);
}

void MainWindow::onOpenFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit openFileRequested(file);
}

void MainWindow::onSaveFile()
{
    emit saveFileRequested();
}

void MainWindow::onSaveFileAs()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save File As"), QString(), tr("Database (*.db)"));
    if (file.isEmpty()) return;
    emit saveFileAsRequested(file);
}
