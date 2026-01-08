#include "core/controllers/ImportController.h"

#include "core/controllers/StatementController.h"
#include "core/models/Statement.h"

namespace debug { std::string startRun(const std::string& processName); void endRun(); }

ImportController::ImportController(std::shared_ptr<StatementController> statementController)
    : statementController_(std::move(statementController))
{
}

std::shared_ptr<Statement> ImportController::import(ImportType type, const std::string& filePath, const std::string& runRoot, const std::string& runIdPrefix, std::function<void(double, const std::string&)> progressCallback, std::shared_ptr<std::atomic<bool>> cancelFlag)
{
    try { debug::startRun("import"); } catch (...) {}

    std::shared_ptr<Statement> result;

    try {
        switch (type) {
        case ImportType::Statement:
            if (statementController_) {
                result = statementController_->importStatement(filePath, runRoot, runIdPrefix, std::move(progressCallback), cancelFlag);
            }
            break;
        }
    } catch (...) {
        try { debug::endRun(); } catch (...) {}
        throw;
    }

    try { debug::endRun(); } catch (...) {}
    return result;
}
