#include <QApplication>
#include "pch.h"
#include "views/MainWindow.h"
#include "llama/LlamaEngine.h"
#include "controllers/StatementController.h"
#include "utils/Env.h"
#include <memory>

int main(int argc, char* argv[]) {
    bool loaded = env::load_dotenv("C:/coding/fossredder/.env", false);
    Q_UNUSED(loaded);

    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}