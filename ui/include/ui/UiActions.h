#pragma once

#include <QObject>

class MainWindow;

class UiActions : public QObject {
    Q_OBJECT
public:
    explicit UiActions(MainWindow* window);

    Q_INVOKABLE void importFile();
    Q_INVOKABLE void newFile();
    Q_INVOKABLE void openFile();
    Q_INVOKABLE void saveFile();
    Q_INVOKABLE void saveFileAs();

private:
    MainWindow* m_window;
};
