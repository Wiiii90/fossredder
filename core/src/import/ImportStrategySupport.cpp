/**
 * @file core/src/import/ImportStrategySupport.cpp
 * @brief Implements private support types and helpers for the default import strategy.
 */

#include "ImportStrategySupport.h"

#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporting.h"
#include "core/jobs/Scheduler.h"
#include "../utils/UniqId.h"

#include <future>
#include <nlohmann/json.hpp>

namespace core::importing {

SchedulerResources::SchedulerResources(const ImportRequest& req)
    : localScheduler(core::constants::importing::kLocalSchedulerWorkers,
                     core::constants::importing::kLocalSchedulerQueueCapacity)
    , localOcrLimiter(core::constants::importing::kLocalOcrSlots)
    , scheduler(req.scheduler ? req.scheduler : &localScheduler)
    , ocrLimiter(req.ocrLimiter ? req.ocrLimiter : &localOcrLimiter)
{
}

void ensureDirectoryExists(const std::filesystem::path& path,
                          core::errors::IErrorReporter* errorReporter,
                          const char* origin)
{
    try {
        std::filesystem::create_directories(path);
    } catch (...) {
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, origin, std::current_exception());
    }
}

detail::ProgressReporter makeProgressReporter(const ImportRequest& req,
                                              core::errors::IErrorReporter* errorReporter)
{
    return [&](double progress, const std::string& phase) {
        if (!req.progressCallback) return;

        try {
            req.progressCallback(progress, phase);
        } catch (...) {
            core::errors::reportException(errorReporter,
                                          core::errors::ErrorSeverity::Warning,
                                          "core::import::DefaultImportStatementStrategy::progressCallback",
                                          std::current_exception());
        }
    };
}

api::poppler::RenderRequest makeRenderRequest(const ImportRequest& req)
{
    api::poppler::RenderRequest request;
    request.pdfPath = std::filesystem::path(req.sourcePath);
    request.dpi = core::constants::importing::kRenderDpi;
    request.outputDir = std::filesystem::path();
    request.uniqIdPrefix = utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kPopplerRenderPrefix);
    request.cancelFlag = req.cancelFlag;
    return request;
}

api::poppler::ExtractRequest makeExtractRequest(const api::poppler::RenderRequest& renderRequest,
                                                const ImportRequest& req)
{
    api::poppler::ExtractRequest request;
    request.pdfPath = renderRequest.pdfPath;
    request.dpi = renderRequest.dpi;
    request.outputDir = std::filesystem::path();
    request.uniqIdPrefix = utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kPopplerExtractPrefix);
    request.cancelFlag = req.cancelFlag;
    return request;
}

std::vector<detail::PageWork> collectPageWork(const ImportRequest& req,
                                              const api::poppler::RenderResult& renderResult,
                                              const api::poppler::ExtractResult& extractResult,
                                              const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                              const std::shared_ptr<api::tesseract::ITesseractService>& tesseract,
                                              SchedulerResources& resources,
                                              const detail::ProgressReporter& report,
                                              core::errors::IErrorReporter* errorReporter,
                                              ImportResult& out,
                                              std::mutex& artifactsMutex)
{
    const size_t totalPages = std::max(renderResult.images.size(), renderResult.imageBytes.size());
    static constexpr size_t unitsPerPage = core::constants::importing::kUnitsPerPage;
    const size_t totalUnits = std::max<size_t>(1, totalPages * unitsPerPage);

    std::atomic<size_t> doneUnits{0};
    std::vector<detail::PageWork> pages(totalPages);
    std::vector<std::future<detail::PageWork>> futures;
    futures.reserve(totalPages);

    for (size_t pageIndex = 0; pageIndex < totalPages; ++pageIndex) {
        auto promise = std::make_shared<std::promise<detail::PageWork>>();
        futures.push_back(promise->get_future());

        resources.scheduler->enqueue([&, pageIndex, promise]() mutable {
            promise->set_value(detail::processImportPage(pageIndex,
                                                         totalPages,
                                                         req,
                                                         renderResult,
                                                         extractResult,
                                                         opencv,
                                                         tesseract,
                                                         resources.ocrLimiter,
                                                         report,
                                                         doneUnits,
                                                         totalUnits,
                                                         errorReporter,
                                                         out,
                                                         artifactsMutex));
        });
    }

    for (auto& future : futures) {
        try {
            auto pageWork = future.get();
            if (pageWork.pageIndex < pages.size()) pages[pageWork.pageIndex] = std::move(pageWork);
        } catch (...) {
            core::errors::reportException(errorReporter,
                                          core::errors::ErrorSeverity::Warning,
                                          "core::import::DefaultImportStatementStrategy::pageFutureGet",
                                          std::current_exception());
        }
    }

    return pages;
}

std::filesystem::path createProofDir(const std::filesystem::path& runRoot,
                                     const std::shared_ptr<IDebugger>& debugger,
                                     core::errors::IErrorReporter* errorReporter)
{
    if (!debugger || !debugger->enabled()) return {};

    const auto proofDir = runRoot / "proof";
    ensureDirectoryExists(proofDir, errorReporter, "core::import::DefaultImportStatementStrategy::createProofDir");
    return proofDir;
}

void attachMetricsArtifact(ImportResult& out,
                           const ImportRequest& req,
                           const std::vector<detail::PageWork>& pages,
                           size_t totalPages,
                           const detail::FinalizeStats& finalizeStats,
                           const ImportRunTimings& timings,
                           core::errors::IErrorReporter* errorReporter)
{
    try {
        nlohmann::json metrics;
        metrics[core::constants::importing::metrics::kJobId] = req.jobId;
        metrics[core::constants::importing::metrics::kSourcePath] = req.sourcePath;
        metrics[core::constants::importing::metrics::kPagesTotal] = totalPages;
        metrics[core::constants::importing::metrics::kPagesWithTable] = finalizeStats.pagesWithTable;
        metrics[core::constants::importing::metrics::kRenderSeconds] = timings.renderSec;
        metrics[core::constants::importing::metrics::kExtractSeconds] = timings.extractSec;
        metrics[core::constants::importing::metrics::kFinalizeSeconds] = timings.finalizeSec;
        metrics[core::constants::importing::metrics::kPageWorkSecondsMax] = finalizeStats.maxPageSec;
        metrics[core::constants::importing::metrics::kPageWorkSecondsSum] = finalizeStats.sumPageSec;
        metrics[core::constants::importing::metrics::kOcrSecondsSum] = finalizeStats.sumOcrSec;
        metrics[core::constants::importing::metrics::kOcrWordsTotal] = finalizeStats.totalOcrWords;
        metrics[core::constants::importing::metrics::kTotalSeconds] = std::chrono::duration<double>(ImportClock::now() - timings.startedAt).count();

        auto& pageArray = metrics[core::constants::importing::metrics::kPages];
        pageArray = nlohmann::json::array();
        for (const auto& page : pages) {
            nlohmann::json pageJson;
            pageJson[core::constants::importing::metrics::kIndex] = page.pageIndex;
            pageJson[core::constants::importing::metrics::kHasTable] = page.hasTable;
            pageJson[core::constants::importing::metrics::kTotalSeconds] = page.totalSec;
            pageJson[core::constants::importing::metrics::kOcrSecondsSum] = page.ocrSec;
            pageJson[core::constants::importing::metrics::kOcrWords] = page.ocrWords;
            pageArray.push_back(std::move(pageJson));
        }

        const auto serialized = metrics.dump(2);
        out.artifacts[std::string(core::constants::importing::kMetricsArtifactName)] =
            std::vector<uint8_t>(serialized.begin(), serialized.end());
    } catch (...) {
        core::errors::reportException(errorReporter,
                                      core::errors::ErrorSeverity::Warning,
                                      "core::import::DefaultImportStatementStrategy::metricsArtifact",
                                      std::current_exception());
    }
}

} // namespace core::importing
