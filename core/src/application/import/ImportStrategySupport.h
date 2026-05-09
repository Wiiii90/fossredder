/**
 * @file core/src/application/import/ImportStrategySupport.h
 * @brief Declares private support types and helpers for the default import strategy.
 */

#pragma once

#include "ImportPipelineHelpers.h"

#include "core/ports/services/IOpenCvService.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResult.h"
#include "core/ports/services/ITesseractService.h"
#include "core/errors/IErrorReporter.h"
#include "core/jobs/Scheduler.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

namespace core::application::importing {

/**
 * @brief Clock type used for import timing measurements.
 */
using ImportClock = std::chrono::steady_clock;

/**
 * @brief Collects timing values for the major import phases.
 */
struct ImportRunTimings {
    ImportClock::time_point startedAt = ImportClock::now();
    double renderSec = 0.0;
    double extractSec = 0.0;
    double finalizeSec = 0.0;
};

/**
 * @brief Owns local scheduler resources when the import request does not provide them.
 */
struct SchedulerResources {
    core::jobs::Scheduler localScheduler;
    core::jobs::SlotLimiter localOcrLimiter;
    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;

    explicit SchedulerResources(const ImportRequest& req);
};

/**
 * @brief Ensures that a target directory exists before import output is written.
 * @param path Directory path to create.
 * @param errorReporter Optional error reporter.
 * @param origin String describing the caller for diagnostics.
 */
void ensureDirectoryExists(const std::filesystem::path& path,
                          core::errors::IErrorReporter* errorReporter,
                          const char* origin);

/**
 * @brief Creates the progress reporter used by the import strategy.
 * @param req Current import request.
 * @param errorReporter Optional error reporter.
 * @return Progress reporter closure.
 */
internal::ProgressReporter makeProgressReporter(const ImportRequest& req,
                                                core::errors::IErrorReporter* errorReporter);

/**
 * @brief Creates the Poppler render request for the current import run.
 * @param req Current import request.
 * @return Render request ready for Poppler.
 */
api::poppler::RenderRequest makeRenderRequest(const ImportRequest& req);
/**
 * @brief Creates the Poppler text-extraction request for the current import run.
 * @param renderRequest Poppler render request that produced the input pages.
 * @param req Current import request.
 * @return Extraction request ready for Poppler.
 */
api::poppler::ExtractRequest makeExtractRequest(const api::poppler::RenderRequest& renderRequest,
                                                const ImportRequest& req);

/**
 * @brief Collects page work for all rendered pages in the import run.
 * @param req Current import request.
 * @param renderResult Poppler render result.
 * @param extractResult Poppler extraction result.
 * @param opencv OpenCV service used for image processing.
 * @param tesseract Tesseract service used for OCR.
 * @param resources Scheduler resources used by the pipeline.
 * @param report Progress callback used by the pipeline.
 * @param errorReporter Optional error reporter.
 * @param out Import result receiving page artifacts.
 * @param artifactsMutex Mutex protecting artifact accumulation.
 * @return Page work collection.
 */
std::vector<internal::PageWork> collectPageWork(const ImportRequest& req,
                                              const api::poppler::RenderResult& renderResult,
                                              const api::poppler::ExtractResult& extractResult,
                                              const std::shared_ptr<core::ports::services::IOpenCvService>& opencv,
                                              const std::shared_ptr<core::ports::services::ITesseractService>& tesseract,
                                              SchedulerResources& resources,
                                              const internal::ProgressReporter& report,
                                              core::errors::IErrorReporter* errorReporter,
                                              ImportResult& out,
                                              std::mutex& artifactsMutex);

/**
 * @brief Attaches aggregated runtime metrics to the import result.
 * @param out Import result receiving the metrics artifact.
 * @param req Current import request.
 * @param pages Parsed page work items.
 * @param totalPages Total page count.
 * @param finalizeStats Finalization statistics.
 * @param timings Phase timing values.
 * @param errorReporter Optional error reporter.
 */
void attachMetricsArtifact(ImportResult& out,
                           const ImportRequest& req,
                           const std::vector<internal::PageWork>& pages,
                           size_t totalPages,
                           const internal::FinalizeStats& finalizeStats,
                           const ImportRunTimings& timings,
                           core::errors::IErrorReporter* errorReporter);

} // namespace core::application::importing
