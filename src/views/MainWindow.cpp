#include "views/MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FOSSredder");
    resize(1024, 768);
    // Hier können später Menüs, Toolbars, zentrale Widgets etc. ergänzt werden
}

MainWindow::~MainWindow() = default;