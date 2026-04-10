/**
 * @file ui/include/MainWindow.h
 * @brief Main window declaration for the UI application.
 */

#pragma once

#include <QMainWindow>
#include <QUrl>

#include "ui/config/Defaults.h"
#include "ui/actions/Actions.h"
#include "ui/state/StateFacade.h"
#include "ui/state/StatusState.h"
#include "ui/window/CloseWorkflow.h"
#include "ui/window/DropController.h"

QT_FORWARD_DECLARE_CLASS(QQmlImageProviderBase)
QT_FORWARD_DECLARE_CLASS(QQmlEngine)
QT_FORWARD_DECLARE_CLASS(QQuickView)
QT_FORWARD_DECLARE_CLASS(QWidget)

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setQmlContextProperty(const QString& name, QObject* value);
    void addImageProvider(const QString& id, QQmlImageProviderBase* provider);
    void loadQml(const QUrl& source = QUrl(ui::config::kMainQmlSource));
    QQmlEngine* qmlEngine() const noexcept;
    ui::StateFacade* dataSession() const noexcept { return dataSession_; }

public slots:
    void handleStorageOperationSucceeded(const QString& operation);
    void handleStorageOperationFailed(const QString& operation, const QString& error);

signals:
    void newFileRequested(const QString& path);
    void openFileRequested(const QString& path);
    void saveFileRequested();
    void saveFileAsRequested(const QString& path);

private slots:
    void onAbout();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void setupQuickHost();
    void setupUiContext();
    void setupActionRouting();
    void setupQmlRuntime();
    /** @brief Unloads the hosted QML surface before widget teardown begins. */
    void prepareForQmlShutdown();

    QQuickView* m_quickView = nullptr;
    QWidget* m_quickContainer = nullptr;
    ui::StateFacade* dataSession_ = nullptr;
    ui::Actions* actions_ = nullptr;
    ui::StatusState* status_ = nullptr;
    bool qmlShutdownPrepared_ = false;
    ui::window::CloseWorkflow closeWorkflow_;
    ui::window::DropController dropController_;
};
