#include "MainWindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QEvent>
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
#include <string>

#include "ui/actions/Actions.h"
#include "ui/bootstrap/QmlRuntime.h"
#include "ui/dialogs/FileDialogs.h"
#include "ui/observability/UiTrace.h"
#include "ui/state/StateFacade.h"
#include "ui/state/NavigationState.h"
#include "ui/state/StatusState.h"
#include "ui/controllers/FileSystemController.h"
#include "ui/workflows/FileWorkflow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSRedder");
    resize(1200, 800);

    setupQuickWidget();
    setupUiContext();
    setupActionRouting();

    setCentralWidget(m_quickWidget);

    setupQmlRuntime();
}

void MainWindow::setupQuickWidget()
{
    if (m_quickWidget) return;

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->setMinimumSize(0, 0);

    m_quickWidget->setAcceptDrops(true);

    m_quickWidget->installEventFilter(this);

    ui::bootstrap::registerTypes();
}

void MainWindow::setupUiContext()
{
    auto actions = new ui::Actions(this);
    actions_ = actions;
    auto nav = new ui::NavigationState(this);
    dataSession_ = new ui::StateFacade(this);
    auto fileSys = new ui::FileSystemController(this);
    fileWorkflow_ = new ui::workflows::FileWorkflow(this, this);
    status_ = new ui::StatusState(this);
    status_->setText(QStringLiteral("Ready"));

    if (m_quickWidget->rootContext()) {
        m_quickWidget->rootContext()->setContextProperty("uiActions", actions);
        m_quickWidget->rootContext()->setContextProperty("uiNav", nav);
        m_quickWidget->rootContext()->setContextProperty("uiData", dataSession_);
        m_quickWidget->rootContext()->setContextProperty("fileSystemController", fileSys);
        m_quickWidget->rootContext()->setContextProperty("uiStatus", status_);

#ifdef _DEBUG
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", true);
#else
        m_quickWidget->rootContext()->setContextProperty("isDebugBuild", false);
#endif
    }
}

void MainWindow::setupActionRouting()
{
    if (!actions_) return;

    auto actions = actions_;
    if (fileWorkflow_) {
        connect(actions->newFileAction(), &QAction::triggered, fileWorkflow_, &ui::workflows::FileWorkflow::requestNewFile);
        connect(actions->openFileAction(), &QAction::triggered, fileWorkflow_, &ui::workflows::FileWorkflow::requestOpenFile);
        connect(actions->saveFileAction(), &QAction::triggered, fileWorkflow_, &ui::workflows::FileWorkflow::requestSaveFile);
        connect(actions->saveFileAsAction(), &QAction::triggered, fileWorkflow_, &ui::workflows::FileWorkflow::requestSaveFileAs);

        connect(fileWorkflow_, &ui::workflows::FileWorkflow::newFileRequested, this, [this](const QString& file) {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI requested new file",
                                          {
                                              {"path", file.toStdString()}
                                          });
            emit newFileRequested(file);
        });
        connect(fileWorkflow_, &ui::workflows::FileWorkflow::openFileRequested, this, [this](const QString& file) {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI requested open file",
                                          {
                                              {"path", file.toStdString()}
                                          });
            emit openFileRequested(file);
        });
        connect(fileWorkflow_, &ui::workflows::FileWorkflow::saveFileRequested, this, [this]() {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI requested save file");
            emit saveFileRequested();
        });
        connect(fileWorkflow_, &ui::workflows::FileWorkflow::saveFileAsRequested, this, [this](const QString& file) {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI requested save file as",
                                          {
                                              {"path", file.toStdString()}
                                          });
            emit saveFileAsRequested(file);
        });
    }
    connect(actions->quitAction(), &QAction::triggered, this, &QWidget::close);
    connect(actions->aboutAction(), &QAction::triggered, this, &MainWindow::onAbout);

    connect(actions, &ui::Actions::importBrowseRequested, this, [this, actions](const QString& filter) {
        const QStringList files = ui::dialogs::pickImportFiles(this, filter);
        if (!files.isEmpty()) {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI selected import files",
                                          {
                                              {"count", std::to_string(files.size())},
                                              {"firstFile", files.front().toStdString()}
                                          });
            emit actions->importFilesSelected(files);
            if (files.size() == 1) emit actions->importFileSelected(files.first());
            if (status_) status_->setText(QString("Selected: %1").arg(files.front()));
        }
    });

    connect(actions, &ui::Actions::exportBrowseRequested, this, [this, actions](const QString& filter) {
        const QString file = ui::dialogs::pickExportFile(this, filter);
        if (!file.isEmpty()) {
            ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                          core::errors::codes::UiFlowMainWindowAction,
                                          "MainWindow::setupActionRouting",
                                          "UI selected export path",
                                          {
                                              {"path", file.toStdString()}
                                          });
            emit actions->exportFileSelected(file);
            if (status_) status_->setText(QString("Export path: %1").arg(file));
        }
    });
}

void MainWindow::setupQmlRuntime()
{
    Q_INIT_RESOURCE(qml);
    ui::bootstrap::configureRuntime(m_quickWidget ? m_quickWidget->engine() : nullptr);
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

QQmlEngine* MainWindow::qmlEngine() const noexcept
{
    return m_quickWidget ? m_quickWidget->engine() : nullptr;
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
                ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                              core::errors::codes::UiFlowMainWindowAction,
                                              "MainWindow::eventFilter",
                                              "Import files dropped",
                                              {
                                                  {"count", std::to_string(files.size())},
                                                  {"firstFile", files.front().toStdString()}
                                              });
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
    ui::observability::reportFlow(core::errors::ErrorSeverity::Info,
                                  core::errors::codes::UiFlowMainWindowAction,
                                  "MainWindow::closeEvent",
                                  "Main window close requested; triggering save workflow");
    if (fileWorkflow_) fileWorkflow_->requestSaveFile();
    else emit saveFileRequested();
    QMainWindow::closeEvent(event);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, tr("About FOSSRedder"), tr("FOSSRedder - demo"));
}
