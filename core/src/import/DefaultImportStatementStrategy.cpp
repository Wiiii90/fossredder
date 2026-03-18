/**
 * @file core/src/import/DefaultImportStatementStrategy.cpp
 * @brief Implements the default import strategy orchestration for statement extraction.
 */

#include "core/pch.h"

#include "ImportStatementStrategy.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResponse.h"
#include "api/tesseract/ITesseractService.h"
#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporting.h"
#include "core/import/IImportStatement.h"
#include "ImportPipelineHelpers.h"
#include "ImportStrategySupport.h"
#include "debug/ErrorReporter.h"
#include "debug/IDebugger.h"
#include "core/models/Statement.h"
#include <filesystem>
#include <memory>
#include <mutex>
#include <chrono>

using core::importing::detail::FinalizeStats;
using core::importing::detail::PageWork;
using core::importing::detail::finalizeParsedPages;

namespace core::importing {

class DefaultImportStatementStrategy : public IImportStatementStrategy {
public:
    DefaultImportStatementStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
        std::shared_ptr<api::opencv::IOpenCvService> opencv,
        std::shared_ptr<api::tesseract::ITesseractService> tesseract,
        std::shared_ptr<IDebugger> debugger)
        : poppler_(std::move(poppler))
        , opencv_(std::move(opencv))
        , tesseract_(std::move(tesseract))
        , debugger_(std::move(debugger))
        , errorReporter_(debugger_ ? std::make_shared<debug::DebuggerErrorReporter>(debugger_) : nullptr) {
    }

    ImportResult run(const ImportRequest& req) override {
        ImportResult out;
        if (!poppler_ || !opencv_ || !tesseract_) return out;
        if (req.runRoot.empty()) return out;

        core::importing::ImportRunTimings timings;

        const std::filesystem::path runRoot(req.runRoot);
        core::importing::ensureDirectoryExists(runRoot, errorReporter_.get(), "core::import::DefaultImportStatementStrategy::createRunRoot");

        auto report = core::importing::makeProgressReporter(req, errorReporter_.get());

        report(core::constants::importing::kProgressPreparing, std::string(core::constants::importing::kProgressPreparingMessage));

        Statement stmt;
        std::vector<ImportedTransaction> all;

        const auto renderRequest = core::importing::makeRenderRequest(req);

        core::errors::report(errorReporter_.get(), {
            core::errors::ErrorSeverity::Info,
            "core::import::DefaultImportStatementStrategy::run",
            std::string("poppler render start: ") + renderRequest.pdfPath.string() + " dpi=" + std::to_string(renderRequest.dpi),
            {}
        });
        report(core::constants::importing::kProgressRendering, std::string(core::constants::importing::kProgressRenderingMessage));
        const auto renderStart = core::importing::ImportClock::now();
        auto renderRes = poppler_->render(renderRequest);
        timings.renderSec = std::chrono::duration<double>(core::importing::ImportClock::now() - renderStart).count();
        report(core::constants::importing::kProgressRendered, std::string(core::constants::importing::kProgressRenderedMessage));

        if (req.cancelFlag && req.cancelFlag->load()) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }

        const auto extractRequest = core::importing::makeExtractRequest(renderRequest, req);
        report(core::constants::importing::kProgressExtracting, std::string(core::constants::importing::kProgressExtractingMessage));
        const auto extractStart = core::importing::ImportClock::now();
        auto extractRes = poppler_->extract(extractRequest);
        timings.extractSec = std::chrono::duration<double>(core::importing::ImportClock::now() - extractStart).count();
        report(core::constants::importing::kProgressExtracted, std::string(core::constants::importing::kProgressExtractedMessage));

        std::string carriedBookingDate;
        int nextTxIndex = 1;

        core::importing::SchedulerResources schedulerResources(req);
        std::mutex artifactsMutex;
        auto pages = core::importing::collectPageWork(req,
                                                      renderRes,
                                                      extractRes,
                                                      opencv_,
                                                      tesseract_,
                                                      schedulerResources,
                                                      report,
                                                      errorReporter_.get(),
                                                      out,
                                                      artifactsMutex);

        const size_t totalPages = pages.size();
        const auto proofDir = core::importing::createProofDir(runRoot, debugger_, errorReporter_.get());

        const auto finalizeStart = core::importing::ImportClock::now();
        const auto finalizeStats = finalizeParsedPages(req,
                                                       pages,
                                                       opencv_,
                                                       proofDir,
                                                       out,
                                                       all,
                                                       carriedBookingDate,
                                                       nextTxIndex,
                                                       errorReporter_.get(),
                                                       report,
                                                       artifactsMutex);
        timings.finalizeSec = std::chrono::duration<double>(core::importing::ImportClock::now() - finalizeStart).count();

        if (!all.empty()) {
            out.data = std::make_shared<Statement>(std::move(stmt));
            out.transactions = std::move(all);
        }

        core::importing::attachMetricsArtifact(out, req, pages, totalPages, finalizeStats, timings, errorReporter_.get());

        report(1.0, std::string(core::constants::importing::kProgressDoneMessage));
        return out;
    }

private:
    std::shared_ptr<api::poppler::IPopplerService> poppler_;
    std::shared_ptr<api::opencv::IOpenCvService> opencv_;
    std::shared_ptr<api::tesseract::ITesseractService> tesseract_;
    std::shared_ptr<IDebugger> debugger_;
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
    std::shared_ptr<api::opencv::IOpenCvService> opencv,
    std::shared_ptr<api::tesseract::ITesseractService> tesseract,
    std::shared_ptr<IDebugger> debugger) {
    return std::make_unique<DefaultImportStatementStrategy>(std::move(poppler), std::move(opencv), std::move(tesseract), std::move(debugger));
}

} // namespace core::importing
