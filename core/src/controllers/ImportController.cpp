#include "core/controllers/ImportController.h"

#include "core/controllers/StatementController.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/import/IImportStatement.h"
#include "core/models/Statement.h"

namespace debug { std::string startRun(const std::string& processName); void endRun(); }

ImportController::ImportController(std::shared_ptr<StatementController> statementController)
    : statementController_(std::move(statementController))
{
}

ImportResult ImportController::import(ImportType type,
                                      const std::string& filePath,
                                      const std::string& runRoot,
                                      const std::string& runIdPrefix,
                                      std::function<void(double, const std::string&)> progressCallback,
                                      std::shared_ptr<std::atomic<bool>> cancelFlag,
                                      core::jobs::Scheduler* scheduler,
                                      core::jobs::SlotLimiter* ocrLimiter,
                                      std::string jobId)
{
    try {
        debug::startRun("import");
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::ImportController::import::startRun", std::current_exception());
    }

    ImportResult result;

    try {
        switch (type) {
        case ImportType::Statement:
            if (statementController_) {
                result = statementController_->importStatementWithArtifacts(filePath, runRoot, runIdPrefix, std::move(progressCallback), cancelFlag, scheduler, ocrLimiter, std::move(jobId));
            }
            break;
        }
    } catch (...) {
        try {
            debug::endRun();
        } catch (...) {
            core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::ImportController::import::endRunOnError", std::current_exception());
        }
        throw;
    }

    try {
        debug::endRun();
    } catch (...) {
        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::ImportController::import::endRun", std::current_exception());
    }
    return result;
}
