#include "core/controllers/ImportController.h"

#include "core/controllers/StatementController.h"

namespace debug { std::string startRun(const std::string& processName); void endRun(); }

ImportController::ImportController(std::shared_ptr<StatementController> statementController)
    : statementController_(std::move(statementController))
{
}

void ImportController::import(ImportType type, const std::string& filePath)
{
    try { debug::startRun("import"); } catch (...) {}

    try {
        switch (type) {
        case ImportType::Statement:
            if (statementController_) {
                (void)statementController_->importStatement(filePath);
            }
            break;
        }
    } catch (...) {
        try { debug::endRun(); } catch (...) {}
        throw;
    }

    try { debug::endRun(); } catch (...) {}
}
