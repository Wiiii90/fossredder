/**
 * @file core/src/import/ImportStrategySupport.h
 * @brief Declares private support types and helpers for the default import strategy.
 */

#pragma once

#include "ImportPipelineHelpers.h"

#include "api/opencv/IOpenCvService.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include "api/tesseract/ITesseractService.h"
#include "core/errors/IErrorReporter.h"
#include "core/jobs/Scheduler.h"
#include "debug/IDebugger.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

namespace core::importing {

using ImportClock = std::chrono::steady_clock;

struct ImportRunTimings {
    ImportClock::time_point startedAt = ImportClock::now();
    double renderSec = 0.0;
    double extractSec = 0.0;
    double finalizeSec = 0.0;
};

struct SchedulerResources {
    core::jobs::Scheduler localScheduler;
    core::jobs::SlotLimiter localOcrLimiter;
    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;

    explicit SchedulerResources(const ImportRequest& req);
};

void ensureDirectoryExists(const std::filesystem::path& path,
                          core::errors::IErrorReporter* errorReporter,
                          const char* origin);

detail::ProgressReporter makeProgressReporter(const ImportRequest& req,
                                              core::errors::IErrorReporter* errorReporter);

api::poppler::RenderRequest makeRenderRequest(const ImportRequest& req);
api::poppler::ExtractRequest makeExtractRequest(const api::poppler::RenderRequest& renderRequest,
                                                const ImportRequest& req);

std::vector<detail::PageWork> collectPageWork(const ImportRequest& req,
                                              const api::poppler::RenderResult& renderResult,
                                              const api::poppler::ExtractResult& extractResult,
                                              const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                              const std::shared_ptr<api::tesseract::ITesseractService>& tesseract,
                                              SchedulerResources& resources,
                                              const detail::ProgressReporter& report,
                                              core::errors::IErrorReporter* errorReporter,
                                              ImportResult& out,
                                              std::mutex& artifactsMutex);

std::filesystem::path createProofDir(const std::filesystem::path& runRoot,
                                     const std::shared_ptr<IDebugger>& debugger,
                                     core::errors::IErrorReporter* errorReporter);

void attachMetricsArtifact(ImportResult& out,
                           const ImportRequest& req,
                           const std::vector<detail::PageWork>& pages,
                           size_t totalPages,
                           const detail::FinalizeStats& finalizeStats,
                           const ImportRunTimings& timings,
                           core::errors::IErrorReporter* errorReporter);

} // namespace core::importing
