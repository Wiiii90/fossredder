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
#include <QQuickView>
#include <QSizePolicy>
#include <QWidget>
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

/** @brief Keeps the QML root object's size properties aligned with the host size. */
void syncRootObjectSize(QQuickView *quickView, QWidget *hostWidget) {
  if (!quickView || !hostWidget || !quickView->rootObject())
    return;

  QObject *root = quickView->rootObject();
  root->setProperty(ui::qml::contracts::properties::kWidth,
                    hostWidget->width());
  root->setProperty(ui::qml::contracts::properties::kHeight,
                    hostWidget->height());
}

/** @brief Reports synchronous QML load failures from the quick view. */
void reportQmlLoadErrors(QQuickView *quickView, const QUrl &source) {
  if (!quickView || quickView->status() != QQuickView::Error)
    return;

  const auto errors = quickView->errors();
  if (errors.isEmpty()) {
    core::errors::report(core::errors::ErrorSeverity::Error,
                         ui::observability::codes::QmlLoadFailed,
                         ui::observability::origins::mainWindow::kLoadQml,
                         "QQuickView failed to load the main QML source",
                         {{"url", source.toString().toStdString()}});
    return;
  }

  for (const auto &error : errors) {
    core::errors::report(core::errors::ErrorSeverity::Error,
                         ui::observability::codes::QmlLoadFailed,
                         ui::observability::origins::mainWindow::kLoadQml,
                         error.toString().toStdString(),
                         {{"url", error.url().toString().toStdString()},
                          {"line", std::to_string(error.line())},
                          {"column", std::to_string(error.column())}});
  }
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle(ui::config::kMainWindowTitle);
  resize(ui::config::kMainWindowDefaultWidth,
         ui::config::kMainWindowDefaultHeight);

  setupQuickHost();
  setupUiContext();
  setCentralWidget(m_quickContainer);

  setupActionRouting();

  setupQmlRuntime();
}

void MainWindow::setupQuickHost() {
  if (m_quickView || m_quickContainer)
    return;

  ui::bootstrap::registerTypes();

  m_quickView = new QQuickView();
  m_quickView->setResizeMode(QQuickView::SizeRootObjectToView);

  m_quickContainer = QWidget::createWindowContainer(m_quickView, this);
  m_quickContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_quickContainer->setMinimumSize(0, 0);
  m_quickContainer->setAcceptDrops(true);
  m_quickContainer->installEventFilter(this);
}

void MainWindow::setupUiContext() {
  if (!m_quickView || !m_quickView->rootContext())
    return;

  const auto services =
      ui::window::installMainWindowContext(*m_quickView->rootContext(), this,
                                           this);
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
  ui::bootstrap::configureRuntime(m_quickView ? m_quickView->engine()
                                              : nullptr);
}

void MainWindow::prepareForQmlShutdown() {
  if (qmlShutdownPrepared_ || (!m_quickView && !m_quickContainer))
    return;

  qmlShutdownPrepared_ = true;
  auto *quickView = m_quickView;
  auto *quickContainer = m_quickContainer;
  m_quickView = nullptr;
  m_quickContainer = nullptr;

  if (quickContainer)
    quickContainer->removeEventFilter(this);
  if (quickView && !quickView->source().isEmpty())
    quickView->setSource(QUrl());
  if (quickContainer && centralWidget() == quickContainer)
    takeCentralWidget();
  if (quickContainer) {
    quickContainer->hide();
    quickContainer->setParent(nullptr);
    delete quickContainer;
  } else if (quickView) {
    delete quickView;
  }
}

MainWindow::~MainWindow() { prepareForQmlShutdown(); }

void MainWindow::setQmlContextProperty(const QString &name, QObject *value) {
  if (!m_quickView)
    return;
  if (!m_quickView->rootContext())
    return;
  m_quickView->rootContext()->setContextProperty(name, value);
}

void MainWindow::addImageProvider(const QString &id,
                                  QQmlImageProviderBase *provider) {
  if (!m_quickView)
    return;
  if (!m_quickView->engine())
    return;
  m_quickView->engine()->addImageProvider(id, provider);
}

void MainWindow::loadQml(const QUrl &source) {
  if (!m_quickView)
    return;
  if (m_quickView->source() == source)
    return;

  m_quickView->setSource(source);
  reportQmlLoadErrors(m_quickView, source);
  syncRootObjectSize(m_quickView, m_quickContainer);
}

QQmlEngine *MainWindow::qmlEngine() const noexcept {
  return m_quickView ? m_quickView->engine() : nullptr;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev) {
  if (obj == m_quickContainer && ev->type() == QEvent::Resize) {
    syncRootObjectSize(m_quickView, m_quickContainer);
  }

  if (obj == m_quickContainer) {
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
    prepareForQmlShutdown();
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
