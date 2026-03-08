#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <QString>

#include "core/jobs/JobManager.h"

class Statement;

namespace core::jobs {
class JobSystem;
}

namespace ui::importing {

using ExceptionReporter = std::function<void(const char* origin, std::exception_ptr exception)>;

class ImportJobBridge {
public:
    explicit ImportJobBridge(std::shared_ptr<core::jobs::JobSystem> jobSystem);
    ~ImportJobBridge();

    bool startStatementImport(const core::jobs::ImportStatementJobSpec& spec,
                              core::jobs::JobEventCallback callback,
                              const ExceptionReporter& exceptionReporter);

    bool isAvailable() const noexcept { return static_cast<bool>(jobSystem_); }

    void cancelCurrent();
    void clearSubscription(const ExceptionReporter& exceptionReporter);

    std::shared_ptr<Statement> statementResult() const;
    std::map<std::string, std::vector<uint8_t>> takeArtifacts();

private:
    std::shared_ptr<core::jobs::JobSystem> jobSystem_;
    QString currentJobId_;
    std::uint64_t currentSubId_ = 0;
};

}