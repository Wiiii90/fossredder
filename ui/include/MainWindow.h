#pragma once

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QQuickWidget)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onImport();
    void onExport();
    void onAbout();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    QQuickWidget* m_quickWidget = nullptr;
};
