/**
 * @file core/src/application/import/ImportPipelineHelpers.h
 * @brief Declares private helper types and functions for the import pipeline implementation.
 */

#pragma once

#include "api/opencv/OpenCvResult.h"
#include "api/poppler/PopplerResult.h"
#include "api/tesseract/TesseractResult.h"
#include "core/errors/IErrorReporter.h"
#include "core/application/import/ImportRequest.h"
#include "core/application/import/ImportResult.h"
#include "core/jobs/Scheduler.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/ports/services/IOpenCvService.h"
#include "core/ports/services/ITesseractService.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace core::application::importing::internal {

/**
 * @brief Function type used to report progress for import pipeline steps.
 */
using ProgressReporter = std::function<void(double, const std::string&)>;

/**
 * @brief Carries all page-local work artifacts gathered during import processing.
 */
struct PageWork {
    bool hasTable = false;
    api::opencv::Table table;
    api::tesseract::ExtractResult ocr;
    std::vector<uint8_t> cropBytes;
    size_t pageIndex = 0;
    double totalSec = 0.0;
    double ocrSec = 0.0;
    size_t ocrWords = 0;
};

/**
 * @brief Aggregates statistics collected during import finalization.
 */
struct FinalizeStats {
    size_t pagesWithTable = 0;
    size_t totalOcrWords = 0;
    double maxPageSec = 0.0;
    double sumPageSec = 0.0;
    double sumOcrSec = 0.0;
};

/**
 * @brief Reads the raw bytes for an import file.
 * @param path Path to the import file.
 * @return File contents as raw bytes.
 */
std::vector<uint8_t> readImportBytes(const std::filesystem::path& path);
/**
 * @brief Reads the raw bytes for an import file and reports errors when requested.
 * @param path Path to the import file.
 * @param errorReporter Optional error reporter.
 * @return File contents as raw bytes.
 */
std::vector<uint8_t> readImportBytes(const std::filesystem::path& path, core::errors::IErrorReporter* errorReporter);

/**
 * @brief Processes one page of import work from rendering to OCR and table detection.
 * @param pageIndex Zero-based page index.
 * @param totalPages Total number of pages in the import document.
 * @param req Current import request.
 * @param renderRes Poppler render result for the document.
 * @param extractRes Poppler text extraction result for the document.
 * @param opencv OpenCV service used for image processing.
 * @param tesseract Tesseract service used for OCR.
 * @param ocrLimiter Optional OCR concurrency limiter.
 * @param report Progress callback used by the pipeline.
 * @param doneUnits Counter tracking finished work units.
 * @param totalUnits Total number of work units.
 * @param errorReporter Optional error reporter.
 * @param out Import result receiving the page artifacts.
 * @param artifactsMutex Mutex protecting artifact accumulation.
 * @return Per-page work summary.
 */
PageWork processImportPage(size_t pageIndex,
                           size_t totalPages,
                           const ImportRequest& req,
                           const api::poppler::RenderResult& renderRes,
                           const api::poppler::ExtractResult& extractRes,
                           const std::shared_ptr<core::ports::services::IOpenCvService>& opencv,
                           const std::shared_ptr<core::ports::services::ITesseractService>& tesseract,
                           core::jobs::SlotLimiter* ocrLimiter,
                           const ProgressReporter& report,
                           std::atomic<size_t>& doneUnits,
                           size_t totalUnits,
                           core::errors::IErrorReporter* errorReporter,
                           ImportResult& out,
                           std::mutex& artifactsMutex);

/**
 * @brief Finalizes parsed pages and converts them into the final import output.
 * @param req Current import request.
 * @param pages Parsed page work items.
 * @param opencv OpenCV service used for artifact generation.
 * @param out Import result receiving the finalized output.
 * @param all Accumulator for all transaction drafts.
 * @param carriedBookingDate Current carried booking date.
 * @param nextTxIndex Next transaction index to assign.
 * @param errorReporter Optional error reporter.
 * @param report Progress callback used by the pipeline.
 * @param artifactsMutex Mutex protecting artifact accumulation.
 * @return Finalization statistics.
 */
FinalizeStats finalizeParsedPages(const ImportRequest& req,
                                  const std::vector<PageWork>& pages,
                                  const std::shared_ptr<core::ports::services::IOpenCvService>& opencv,
                                  ImportResult& out,
                                  std::vector<core::application::importing::draft::TransactionDraft>& all,
                                  std::string& carriedBookingDate,
                                  int& nextTxIndex,
                                  core::errors::IErrorReporter* errorReporter,
                                  const ProgressReporter& report,
                                  std::mutex& artifactsMutex);

} // namespace core::application::importing::internal
