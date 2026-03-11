#include "core/import/ImportPipelineHelpers.h"

#include "core/constants/CoreDefaults.h"
#include "core/errors/ErrorReporting.h"
#include "core/parser/DefaultStatementParser.h"
#include "core/utils/UniqId.h"

#include <algorithm>
#include <fstream>

namespace core::importing::detail {

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path)
{
    return readImportBytes(path, nullptr);
}

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path, core::errors::IErrorReporter* errorReporter)
{
    std::vector<uint8_t> out;
    try {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) return out;
        out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::readBytes", std::current_exception()); }
    return out;
}

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
                           std::mutex& artifactsMutex)
{
    const auto pageStart = std::chrono::steady_clock::now();
    PageWork page;
    page.pageIndex = pageIndex;

    static constexpr size_t unitsPerPage = core::constants::importing::kUnitsPerPage;
    size_t localUnitsDone = 0;
    auto unitDone = [&](size_t inc, const std::string& label) {
        localUnitsDone += inc;
        const size_t d = doneUnits.fetch_add(inc) + inc;
        const double frac = std::min(1.0, static_cast<double>(d) / static_cast<double>(totalUnits));
        const double progress = core::constants::importing::kProgressPageWorkBase + core::constants::importing::kProgressPageWorkSpan * frac;
        report(progress, std::string("[") + std::to_string(pageIndex + 1) + "/" + std::to_string(totalPages) + "] " + label);
    };
    auto finishUnits = [&](const std::string& label) {
        if (localUnitsDone >= unitsPerPage) return;
        unitDone(unitsPerPage - localUnitsDone, label);
    };

    if (req.cancelFlag && req.cancelFlag->load()) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }

    std::vector<uint8_t> pageBytes;
    if (pageIndex < renderRes.imageBytes.size() && !renderRes.imageBytes[pageIndex].empty()) {
        pageBytes = renderRes.imageBytes[pageIndex];
    } else if (pageIndex < renderRes.images.size() && !renderRes.images[pageIndex].empty()) {
        pageBytes = readImportBytes(renderRes.images[pageIndex]);
    }
    if (pageBytes.empty()) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoImage));
        return page;
    }

    api::opencv::MaskRequest maskRequest;
    maskRequest.imageBytes = pageBytes;
    maskRequest.uniqIdPrefix = utils::makeUniqId();
    maskRequest.filePrefix = std::string(core::constants::importing::kOpenCvMaskPrefix) + std::to_string(pageIndex + 1);
    maskRequest.usePoppler = true;
    maskRequest.useMorphology = true;
    maskRequest.useTesseract = false;
    maskRequest.cancelFlag = req.cancelFlag;

    if (pageIndex < extractRes.pages.size()) {
        const auto& pageExtract = extractRes.pages[pageIndex];
        const double scaleX = pageExtract.dpiX / 72.0;
        const double scaleY = pageExtract.dpiY / 72.0;
        for (const auto& textElement : pageExtract.textElements) {
            api::opencv::Rect rect;
            rect.x = static_cast<int>(std::round(textElement.x * scaleX));
            rect.y = static_cast<int>(std::round(textElement.y * scaleY));
            rect.width = static_cast<int>(std::round(textElement.width * scaleX));
            rect.height = static_cast<int>(std::round(textElement.height * scaleY));
            if (rect.width <= 1 || rect.height <= 1) continue;
            maskRequest.textElements.push_back(rect);
        }
    }

    if (maskRequest.textElements.empty()) maskRequest.useTesseract = true;

    if (maskRequest.useTesseract) {
        try {
            api::tesseract::ExtractRequest tessRequest;
            tessRequest.imageBytes = pageBytes;
            tessRequest.tessdataPath = {};
            tessRequest.psm = core::constants::importing::kDefaultTesseractPsm;
            tessRequest.cancelFlag = req.cancelFlag;

            if (ocrLimiter) ocrLimiter->acquire();
            const auto ocrStart = std::chrono::steady_clock::now();
            auto tessResponse = tesseract->extract(tessRequest);
            page.ocrSec += std::chrono::duration<double>(std::chrono::steady_clock::now() - ocrStart).count();
            if (ocrLimiter) ocrLimiter->release();

            maskRequest.tesseractTsv = tessResponse.tsv;
        } catch (...) {
            try { if (ocrLimiter) ocrLimiter->release(); } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseMask", std::current_exception()); }
            core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractMaskExtract", std::current_exception());
        }
    }

    auto maskResponse = opencv->mask(maskRequest);
    std::vector<uint8_t> maskedBytes = !maskResponse.maskedImageBytes.empty() ? maskResponse.maskedImageBytes : pageBytes;
    unitDone(1, std::string(core::constants::importing::pageSteps::kMask));

    api::opencv::DetectRequest detectRequest;
    detectRequest.imageBytes = maskedBytes;
    detectRequest.uniqIdPrefix = utils::makeUniqId();
    detectRequest.filePrefix = std::string(core::constants::importing::kOpenCvDetectPrefix) + std::to_string(pageIndex + 1);
    detectRequest.kind = api::opencv::DetectRequest::DetectKind::Tables;
    detectRequest.cancelFlag = req.cancelFlag;
    auto detectResponse = opencv->detect(detectRequest);
    unitDone(1, std::string(core::constants::importing::pageSteps::kDetect));

    if (!detectResponse.detected) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoTable));
        return page;
    }

    api::opencv::CropRequest cropRequest;
    cropRequest.imageBytes = pageBytes;
    cropRequest.uniqIdPrefix = utils::makeUniqId();
    cropRequest.filePrefix = std::string(core::constants::importing::kOpenCvCropPrefix) + std::to_string(pageIndex + 1);
    cropRequest.bbox = detectResponse.table.bbox;
    cropRequest.cancelFlag = req.cancelFlag;
    auto cropResponse = opencv->crop(cropRequest);
    unitDone(1, std::string(core::constants::importing::pageSteps::kCrop));

    if (cropResponse.croppedImageBytes.empty() || cropResponse.croppedImageBytes.front().empty()) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoCrop));
        return page;
    }

    api::tesseract::ExtractRequest tableRequest;
    tableRequest.kind = api::tesseract::ExtractRequest::Kind::Table;
    tableRequest.imageBytes = cropResponse.croppedImageBytes.front();
    tableRequest.uniqIdPrefix = utils::makeUniqId();
    tableRequest.filePrefix = std::string(core::constants::importing::kTableTesseractPrefix) + std::to_string(pageIndex + 1);
    tableRequest.tessdataPath = {};
    tableRequest.psm = core::constants::importing::kDefaultTableTesseractPsm;
    tableRequest.cancelFlag = req.cancelFlag;

    tableRequest.cells.reserve(detectResponse.table.cells.size());
    for (const auto& cell : detectResponse.table.cells) {
        api::tesseract::Cell mappedCell;
        mappedCell.row = cell.row;
        mappedCell.col = cell.col;
        mappedCell.bbox.x = cell.bbox.x - detectResponse.table.bbox.x;
        mappedCell.bbox.y = cell.bbox.y - detectResponse.table.bbox.y;
        mappedCell.bbox.width = cell.bbox.width;
        mappedCell.bbox.height = cell.bbox.height;
        tableRequest.cells.push_back(mappedCell);
    }

    try {
        if (ocrLimiter) ocrLimiter->acquire();
        const auto ocrStart = std::chrono::steady_clock::now();
        page.ocr = tesseract->extract(tableRequest);
        page.ocrSec += std::chrono::duration<double>(std::chrono::steady_clock::now() - ocrStart).count();
        if (ocrLimiter) ocrLimiter->release();
    } catch (...) {
        try { if (ocrLimiter) ocrLimiter->release(); } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseTable", std::current_exception()); }
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractTableExtract", std::current_exception());
        finishUnits(std::string(core::constants::importing::pageSteps::kOcrFailed));
        return page;
    }

    unitDone(1, std::string(core::constants::importing::pageSteps::kOcr));

    page.hasTable = true;
    page.table = detectResponse.table;
    page.cropBytes = std::move(cropResponse.croppedImageBytes.front());
    page.ocrWords = page.ocr.words.size();

    try {
        std::string tsvKey = "tesseract/page_" + std::to_string(pageIndex) + "_crop_0.tsv";
        std::vector<uint8_t> tsvData(page.ocr.tsv.begin(), page.ocr.tsv.end());
        std::lock_guard<std::mutex> guard(artifactsMutex);
        out.artifacts[tsvKey] = std::move(tsvData);
    } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::storeTsvArtifact", std::current_exception()); }

    finishUnits(std::string(core::constants::importing::pageSteps::kDone));
    page.totalSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - pageStart).count();
    return page;
}

FinalizeStats finalizeParsedPages(const ImportRequest& req,
                                  const std::vector<PageWork>& pages,
                                  const std::shared_ptr<api::opencv::IOpenCvService>& opencv,
                                  const std::filesystem::path& proofDir,
                                  ImportResult& out,
                                  std::vector<ImportedTransaction>& all,
                                  std::string& carriedBookingDate,
                                  int& nextTxIndex,
                                  core::errors::IErrorReporter* errorReporter,
                                  const ProgressReporter& report,
                                  std::mutex& artifactsMutex)
{
    FinalizeStats stats;
    const size_t finalizePageCount = std::max<size_t>(1, pages.size());
    for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
        if (req.cancelFlag && req.cancelFlag->load()) {
            report(0.0, std::string(core::constants::importing::kProgressCanceled));
            break;
        }

        const auto& page = pages[pageIndex];
        if (!page.hasTable) continue;

        const double fraction = static_cast<double>(pageIndex + 1) / static_cast<double>(finalizePageCount);
        const double progress = core::constants::importing::kProgressFinalizeBase + core::constants::importing::kProgressFinalizeSpan * fraction;
        report(progress, std::string("[") + std::to_string(pageIndex + 1) + "/" + std::to_string(finalizePageCount) + "] " + std::string(core::constants::importing::pageSteps::kFinalize));

        ++stats.pagesWithTable;
        stats.totalOcrWords += page.ocrWords;
        stats.maxPageSec = std::max(stats.maxPageSec, page.totalSec);
        stats.sumPageSec += page.totalSec;
        stats.sumOcrSec += page.ocrSec;

        auto parsed = DefaultStatementParser::parse(page.table,
                                                    page.ocr,
                                                    std::string(),
                                                    opencv,
                                                    page.cropBytes,
                                                    proofDir,
                                                    carriedBookingDate,
                                                    nextTxIndex);
        carriedBookingDate = parsed.lastBookingDate;
        nextTxIndex = parsed.nextTransactionIndex;

        if (!parsed.transactions.empty()) {
            for (auto& transaction : parsed.transactions) all.push_back(std::move(transaction));
        }

        if (!parsed.artifacts.empty()) {
            try {
                std::lock_guard<std::mutex> guard(artifactsMutex);
                for (auto& [key, value] : parsed.artifacts) out.artifacts.emplace(key, std::move(value));
            } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::mergeParsedArtifacts", std::current_exception()); }
        }
    }

    return stats;
}

}
