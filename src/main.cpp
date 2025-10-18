#include <QApplication>
#include "pch.h"
#include "views/MainWindow.h"
#include "llama/LlamaEngine.h"
#include "controllers/PdfImportController.h"
#include <memory>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}