/**
 * @file ui/src/MainWindow.cpp
 * @brief Implements the desktop main window hosting the QML UI surface.
 */

#include "MainWindow.h"

#include <QCloseEvent>
#include <QEvent>
#include <QMessageBox>
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlImageProviderBase>
#include <QQuickItem>
#include <QQuickWidget>
#include <QSizePolicy>
#include <string>

#include "ui/actions/Actions.h"
#include "ui/bootstrap/QmlContracts.h"
#include "ui/bootstrap/QmlRuntime.h"
#include "ui/config/Defaults.h"
#include "ui/controllers/ControllerContracts.h"
#include "ui/controllers/ControllerStrings.h"
#include "ui/observability/Origins.h"
#include "ui/text/Text.h"
#include "ui/window/MainWindowContext.h"
#include "ui/window/MainWindowTrace.h"
#include "ui/workflows/FileWorkflow.h"

namespace {

using ui::observability::context::kError;

/** @brief Keeps the QML root object's size properties aligned with the widget size. */
void syncRootObjectSize(QQuickWidget *quickWidget) {
  if (!quickWidget || !quickWidget->rootObject())
    return;
  QObject *root = quickWidget->rootObject();
  root->setProperty(ui::qml::contracts::properties::kWidth,
                    quickWidget->width());
  root->setProperty(ui::qml::contracts::properties::kHeight,
                    quickWidget->height());
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle(ui::config::kMainWindowTitle);
  resize(ui::config::kMainWindowDefaultWidth,
         ui::config::kMainWindowDefaultHeight);

  setupQuickWidget();
  setupUiContext();
  setCentralWidget(m_quickWidget);

  setupActionRouting();

  setupQmlRuntime();
}

void MainWindow::setupQuickWidget() {
  if (m_quickWidget)
    return;

  m_quickWidget = new QQuickWidget(this);
  m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_quickWidget->setMinimumSize(0, 0);

  m_quickWidget->setAcceptDrops(true);

  m_quickWidget->installEventFilter(this);

  ui::bootstrap::registerTypes();
}

void MainWindow::setupUiContext() {
  if (!m_quickWidget)
    return;

  const auto services =
      ui::window::installMainWindowContext(*m_quickWidget, this, this);
  actions_ = services.actions;
  dataSession_ = services.dataSession;
  fileWorkflow_ = services.fileWorkflow;
  status_ = services.status;
}

void MainWindow::setupActionRouting() {
  ui::window::wireMainWindowActions(
      *this, {actions_, dataSession_, fileWorkflow_, status_},
      [this]() { onAbout(); });
}

void MainWindow::setupQmlRuntime() {
  Q_INIT_RESOURCE(qml);
  ui::bootstrap::configureRuntime(m_quickWidget ? m_quickWidget->engine()
                                                : nullptr);
}

MainWindow::~MainWindow() = default;

void MainWindow::setQmlContextProperty(const QString &name, QObject *value) {
  if (!m_quickWidget)
    return;
  if (!m_quickWidget->rootContext())
    return;
  m_quickWidget->rootContext()->setContextProperty(name, value);
}

void MainWindow::addImageProvider(const QString &id,
                                  QQmlImageProviderBase *provider) {
  if (!m_quickWidget)
    return;
  if (!m_quickWidget->engine())
    return;
  m_quickWidget->engine()->addImageProvider(id, provider);
}

void MainWindow::loadQml(const QUrl &source) {
  if (!m_quickWidget)
    return;
  if (m_quickWidget->source() == source)
    return;
  m_quickWidget->setSource(source);
  syncRootObjectSize(m_quickWidget);
}

QQmlEngine *MainWindow::qmlEngine() const noexcept {
  return m_quickWidget ? m_quickWidget->engine() : nullptr;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev) {
  if (obj == m_quickWidget && ev->type() == QEvent::Resize) {
    syncRootObjectSize(m_quickWidget);
  }

  if (obj == m_quickWidget) {
    const auto outcome = dropController_.handle(ev);
    if (outcome.handled) {
      if (ev->type() == QEvent::Drop && outcome.accepted && actions_) {
        ui::window::reportMainWindowFlow(
            ui::observability::origins::mainWindow::kDragDrop,
            "Import files dropped", core::errors::ErrorSeverity::Info,
            ui::window::makeFileListContext(outcome.files));
        emit actions_->importFilesDropped(outcome.files);
        if (outcome.files.size() == 1)
          emit actions_->importFileDropped(outcome.files.first());
      }
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (closeWorkflow_.allowImmediateClose(event)) {
    QMainWindow::closeEvent(event);
    return;
  }

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kClose,
      "Main window close requested; triggering save workflow");
  closeWorkflow_.requestClose(event, [this]() {
    if (fileWorkflow_)
      fileWorkflow_->requestSaveFile();
    else
      emit saveFileRequested();
  });
}

void MainWindow::handleStorageOperationSucceeded(const QString &operation) {
  if (!closeWorkflow_.handleStorageOperationSucceeded(
          operation, ui::controllers::contracts::operations::kSaveFile,
          [this]() {
            QMetaObject::invokeMethod(
                this, [this]() { close(); }, Qt::QueuedConnection);
          })) {
    return;
  }

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kCloseSucceeded,
      "Pending close save finished; closing main window");
}

void MainWindow::handleStorageOperationFailed(const QString &operation,
                                              const QString &error) {
  if (!closeWorkflow_.handleStorageOperationFailed(
          operation, ui::controllers::contracts::operations::kSaveFile))
    return;

  const QString message =
      error.isEmpty() ? ui::text::controllerErrors::storageSaveFailed()
                      : error;
  if (status_)
    status_->setText(message);

  ui::window::reportMainWindowFlow(
      ui::observability::origins::mainWindow::kCloseFailed,
      "Pending close save failed; keeping main window open",
      core::errors::ErrorSeverity::Warning,
      {{kError, ui::strings::toStdString(message)}});
}

void MainWindow::onAbout() {
  QMessageBox::about(this, ui::text::mainWindow::aboutTitle(),
                     ui::text::mainWindow::aboutBody());
}
