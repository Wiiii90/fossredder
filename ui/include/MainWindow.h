#pragma once

#include <QMainWindow>
#include <QUrl>

#include "ui/config/Defaults.h"
#include "ui/actions/Actions.h"
#include "ui/state/StateFacade.h"
#include "ui/state/StatusState.h"
#include "ui/window/CloseWorkflow.h"
#include "ui/window/DropController.h"
#include "ui/workflows/FileWorkflow.h"

QT_FORWARD_DECLARE_CLASS(QQuickWidget)
QT_FORWARD_DECLARE_CLASS(QQmlImageProviderBase)
QT_FORWARD_DECLARE_CLASS(QQmlEngine)

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
    void setupQuickWidget();
    void setupUiContext();
    void setupActionRouting();
    void setupQmlRuntime();

    QQuickWidget* m_quickWidget = nullptr;
    ui::StateFacade* dataSession_ = nullptr;
    ui::Actions* actions_ = nullptr;
    ui::workflows::FileWorkflow* fileWorkflow_ = nullptr;
    ui::StatusState* status_ = nullptr;
    ui::window::CloseWorkflow closeWorkflow_;
    ui::window::DropController dropController_;
};
