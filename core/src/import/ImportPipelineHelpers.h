/**
 * @file core/src/import/ImportPipelineHelpers.h
 * @brief Declares private helper types and functions for the import pipeline implementation.
 */

#pragma once

#include "api/opencv/IOpenCvService.h"
#include "api/opencv/Types.h"
#include "api/poppler/PopplerResponse.h"
#include "api/tesseract/ITesseractService.h"
#include "api/tesseract/TesseractResponse.h"
#include "core/errors/IErrorReporter.h"
#include "core/import/ImportRequest.h"
#include "core/import/ImportResult.h"
#include "core/jobs/Scheduler.h"
#include "core/models/TransactionDraft.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace core::importing::detail {

using ProgressReporter = std::function<void(double, const std::string&)>;

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

struct FinalizeStats {
    size_t pagesWithTable = 0;
    size_t totalOcrWords = 0;
    double maxPageSec = 0.0;
    double sumPageSec = 0.0;
    double sumOcrSec = 0.0;
};

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path);
std::vector<uint8_t> readImportBytes(const std::filesystem::path& path, core::errors::IErrorReporter* errorReporter);

PageWork processImportPage(size_t pageIndex,
                           size_t totalPages,
                           const ImportRequest& req,
                           const api::poppler::RenderResult& renderRes,
                           const api::poppler::ExtractResult& extractRes,
                           const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                           const std::shared_ptr<api::tesseract::ITesseractService>& tesseract,
                           core::jobs::SlotLimiter* ocrLimiter,
                           const ProgressReporter& report,
                           std::atomic<size_t>& doneUnits,
                           size_t totalUnits,
                           core::errors::IErrorReporter* errorReporter,
                           ImportResult& out,
                           std::mutex& artifactsMutex);

FinalizeStats finalizeParsedPages(const ImportRequest& req,
                                  const std::vector<PageWork>& pages,
                                  const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                  ImportResult& out,
                                  std::vector<core::domain::TransactionDraft>& all,
                                  std::string& carriedBookingDate,
                                  int& nextTxIndex,
                                  core::errors::IErrorReporter* errorReporter,
                                  const ProgressReporter& report,
                                  std::mutex& artifactsMutex);

}
