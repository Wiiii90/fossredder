#pragma once

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QQuickWidget)

class UiDataSession;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setQmlContextProperty(const QString& name, QObject* value);
    UiDataSession* dataSession() const noexcept { return dataSession_; }

signals:
    void importRequested(const QString& path);

    void newFileRequested(const QString& path);
    void openFileRequested(const QString& path);
    void saveFileRequested();
    void saveFileAsRequested(const QString& path);

private slots:
    void onImport();
    void onExport();
    void onAbout();

    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QQuickWidget* m_quickWidget = nullptr;
    UiDataSession* dataSession_ = nullptr;
};
