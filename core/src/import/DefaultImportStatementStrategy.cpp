#include "core/pch.h"

#include "core/import/IImportStatementStrategy.h"
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
#include "core/errors/DebuggerErrorReporter.h"
#include "core/errors/ErrorReporting.h"
#include "core/import/IImportStatement.h"
#include "core/import/ImportPipelineHelpers.h"
#include "core/parser/DefaultStatementParser.h"
#include "debug/IDebugger.h"
#include "core/models/Statement.h"
#include "core/utils/UniqId.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>
#include <limits>
#include <future>
#include <mutex>
#include <chrono>

#include <nlohmann/json.hpp>

#include "core/jobs/Scheduler.h"

using core::importing::detail::FinalizeStats;
using core::importing::detail::PageWork;
using core::importing::detail::finalizeParsedPages;
using core::importing::detail::processImportPage;
using core::importing::detail::readImportBytes;


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
        , errorReporter_(debugger_ ? std::make_shared<core::errors::DebuggerErrorReporter>(debugger_) : nullptr) {
    }

    ImportResult run(const ImportRequest& req) override {
        ImportResult out;
        if (!poppler_ || !opencv_ || !tesseract_) return out;
        if (req.runRoot.empty()) return out;

        const auto tStart = std::chrono::steady_clock::now();
        double renderSec = 0.0;
        double extractSec = 0.0;
        double finalizeSec = 0.0;

        const std::filesystem::path runRoot(req.runRoot);
        try { std::filesystem::create_directories(runRoot); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::createRunRoot", std::current_exception()); }

        auto report = [&](double p, const std::string& phase) {
            if (req.progressCallback) {
                try { req.progressCallback(p, phase); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::progressCallback", std::current_exception()); }
            }
        };

        report(core::constants::importing::kProgressPreparing, std::string(core::constants::importing::kProgressPreparingMessage));

        Statement stmt;
        std::vector<ImportedTransaction> all;

        api::poppler::RenderRequest rreq;
        rreq.pdfPath = std::filesystem::path(req.sourcePath);
        rreq.dpi = core::constants::importing::kRenderDpi;
        rreq.outputDir = std::filesystem::path(); // keep production pipeline in-memory (Poppler can still emit debug output via debugger)
        rreq.uniqIdPrefix = utils::makeUniqId();
        rreq.filePrefix = std::string(core::constants::importing::kPopplerRenderPrefix);
        rreq.cancelFlag = req.cancelFlag;

        core::errors::report(errorReporter_.get(), {
            core::errors::ErrorSeverity::Info,
            "core::import::DefaultImportStatementStrategy::run",
            std::string("poppler render start: ") + rreq.pdfPath.string() + " dpi=" + std::to_string(rreq.dpi),
            {}
        });
        report(core::constants::importing::kProgressRendering, std::string(core::constants::importing::kProgressRenderingMessage));
        const auto tRender0 = std::chrono::steady_clock::now();
        auto renderRes = poppler_->render(rreq);
        renderSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tRender0).count();
        report(core::constants::importing::kProgressRendered, std::string(core::constants::importing::kProgressRenderedMessage));

        if (req.cancelFlag && req.cancelFlag->load()) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }

        api::poppler::ExtractRequest ereq;
        ereq.pdfPath = rreq.pdfPath;
        ereq.dpi = rreq.dpi;
        ereq.outputDir = std::filesystem::path(); // keep production pipeline in-memory
        ereq.uniqIdPrefix = utils::makeUniqId();
        ereq.filePrefix = std::string(core::constants::importing::kPopplerExtractPrefix);
        ereq.cancelFlag = req.cancelFlag;
        report(core::constants::importing::kProgressExtracting, std::string(core::constants::importing::kProgressExtractingMessage));
        const auto tExtract0 = std::chrono::steady_clock::now();
        auto extractRes = poppler_->extract(ereq);
        extractSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tExtract0).count();
        report(core::constants::importing::kProgressExtracted, std::string(core::constants::importing::kProgressExtractedMessage));

        std::string carriedBookingDate;
        int nextTxIndex = 1;

        core::jobs::Scheduler localScheduler(core::constants::importing::kLocalSchedulerWorkers,
                                             core::constants::importing::kLocalSchedulerQueueCapacity);
        core::jobs::SlotLimiter localOcrLimiter(core::constants::importing::kLocalOcrSlots);
        core::jobs::Scheduler* sched = req.scheduler ? req.scheduler : &localScheduler;
        core::jobs::SlotLimiter* ocrLimiter = req.ocrLimiter ? req.ocrLimiter : &localOcrLimiter;

        const size_t totalPages = std::max(renderRes.images.size(), renderRes.imageBytes.size());
        std::vector<std::future<PageWork>> pageFutures;
        pageFutures.reserve(totalPages);

        static constexpr size_t unitsPerPage = core::constants::importing::kUnitsPerPage;
        const size_t totalUnits = std::max<size_t>(1, totalPages * unitsPerPage);
        std::atomic<size_t> doneUnits{0};
        std::mutex artifactsMutex;

        std::vector<PageWork> pages;
        pages.resize(totalPages);

        for (size_t pi = 0; pi < totalPages; ++pi) {
            auto prom = std::make_shared<std::promise<PageWork>>();
            pageFutures.push_back(prom->get_future());

            sched->enqueue([&, pi, prom]() mutable {
                prom->set_value(processImportPage(pi,
                                                  totalPages,
                                                  req,
                                                  renderRes,
                                                  extractRes,
                                                  opencv_,
                                                  tesseract_,
                                                  ocrLimiter,
                                                  report,
                                                  doneUnits,
                                                  totalUnits,
                                                  errorReporter_.get(),
                                                  out,
                                                  artifactsMutex));
            });
        }

        for (auto& f : pageFutures) {
            try {
                auto pw = f.get();
                if (pw.pageIndex < pages.size()) pages[pw.pageIndex] = std::move(pw);
            } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::pageFutureGet", std::current_exception()); }
        }

        std::filesystem::path proofDir;
        if (debugger_ && debugger_->enabled()) {
            proofDir = runRoot / "proof";
            try { std::filesystem::create_directories(proofDir); } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::createProofDir", std::current_exception()); }
        }

        const auto tFinalize0 = std::chrono::steady_clock::now();
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
        finalizeSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tFinalize0).count();

        if (!all.empty()) {
            out.data = std::make_shared<Statement>(std::move(stmt));
            out.transactions = std::move(all);
        }

        try {
            nlohmann::json j;
            j[core::constants::importing::metrics::kJobId] = req.jobId;
            j[core::constants::importing::metrics::kSourcePath] = req.sourcePath;
            j[core::constants::importing::metrics::kPagesTotal] = totalPages;
            j[core::constants::importing::metrics::kPagesWithTable] = finalizeStats.pagesWithTable;
            j[core::constants::importing::metrics::kRenderSeconds] = renderSec;
            j[core::constants::importing::metrics::kExtractSeconds] = extractSec;
            j[core::constants::importing::metrics::kFinalizeSeconds] = finalizeSec;
            j[core::constants::importing::metrics::kPageWorkSecondsMax] = finalizeStats.maxPageSec;
            j[core::constants::importing::metrics::kPageWorkSecondsSum] = finalizeStats.sumPageSec;
            j[core::constants::importing::metrics::kOcrSecondsSum] = finalizeStats.sumOcrSec;
            j[core::constants::importing::metrics::kOcrWordsTotal] = finalizeStats.totalOcrWords;
            j[core::constants::importing::metrics::kTotalSeconds] = std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count();

            auto& arr = j[core::constants::importing::metrics::kPages];
            arr = nlohmann::json::array();
            for (const auto& pw : pages) {
                nlohmann::json pj;
                pj[core::constants::importing::metrics::kIndex] = pw.pageIndex;
                pj[core::constants::importing::metrics::kHasTable] = pw.hasTable;
                pj[core::constants::importing::metrics::kTotalSeconds] = pw.totalSec;
                pj[core::constants::importing::metrics::kOcrSecondsSum] = pw.ocrSec;
                pj[core::constants::importing::metrics::kOcrWords] = pw.ocrWords;
                arr.push_back(std::move(pj));
            }

            const auto dumped = j.dump(2);
            out.artifacts[std::string(core::constants::importing::kMetricsArtifactName)] = std::vector<uint8_t>(dumped.begin(), dumped.end());
        } catch (...) { core::errors::reportException(errorReporter_.get(), core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::metricsArtifact", std::current_exception()); }

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
