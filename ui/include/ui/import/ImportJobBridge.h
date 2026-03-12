/**
 * @file ui/include/ui/import/ImportJobBridge.h
 * @brief Declares the bridge between the UI import workflow and the core job system.
 */

#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <QString>

#include "core/import/ImportedTransaction.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobTypes.h"

namespace core::domain { class Statement; }

namespace core::jobs {
class JobSystem;
}

namespace ui::importing {

using ExceptionReporter = std::function<void(const char* origin, std::exception_ptr exception)>;

class ImportJobBridge {
public:
    explicit ImportJobBridge(std::shared_ptr<core::jobs::JobSystem> jobSystem);
    ~ImportJobBridge();

    void setExceptionReporter(ExceptionReporter reporter);

    bool startStatementImport(const core::jobs::ImportStatementJobSpec& spec,
                              core::jobs::JobEventCallback callback);

    bool isAvailable() const noexcept { return static_cast<bool>(jobSystem_); }

    void cancelCurrent();
    void clearSubscription();

    std::shared_ptr<core::domain::Statement> statementResult() const;
    std::vector<ImportedTransaction> statementTransactions() const;
    std::map<std::string, std::vector<uint8_t>> takeArtifacts();

private:
    std::shared_ptr<core::jobs::JobSystem> jobSystem_;
    ExceptionReporter exceptionReporter_;
    QString currentJobId_;
    std::uint64_t currentSubId_ = 0;
};

}