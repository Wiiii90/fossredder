#pragma once

#include <QObject>

class MainWindow;

class UiActions : public QObject
{
    Q_OBJECT
public:
    explicit UiActions(MainWindow* window);

    Q_INVOKABLE void importFile();

private:
    MainWindow* m_window;
};
