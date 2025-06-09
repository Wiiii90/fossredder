#pragma once
#include <QMainWindow>

class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget* m_stack = nullptr;
};