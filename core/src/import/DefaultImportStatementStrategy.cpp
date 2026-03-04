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
#include "core/import/IImportStatement.h"
#include "core/errors/ErrorReporterRegistry.h"
#include "core/parser/DefaultStatementParser.h"
#include "../../debug/include/debug/IDebugger.h"
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

 

class DefaultImportStatementStrategy : public IImportStatementStrategy {
public:
    DefaultImportStatementStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
        std::shared_ptr<api::opencv::IOpenCvService> opencv,
        std::shared_ptr<api::tesseract::ITesseractService> tesseract,
        std::shared_ptr<IDebugger> debugger)
        : poppler_(std::move(poppler)), opencv_(std::move(opencv)), tesseract_(std::move(tesseract)), debugger_(std::move(debugger)) {
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
        try { std::filesystem::create_directories(runRoot); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::createRunRoot", std::current_exception()); }

        auto report = [&](double p, const std::string& phase) {
            if (req.progressCallback) {
                try { req.progressCallback(p, phase); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::progressCallback", std::current_exception()); }
            }
        };

        report(0.02, "Preparing import");

        Statement stmt;
        std::vector<std::shared_ptr<Transaction>> all;

        api::poppler::RenderRequest rreq;
        rreq.pdfPath = std::filesystem::path(req.sourcePath);
        rreq.dpi = 300.0;
        rreq.outputDir = std::filesystem::path(); // keep production pipeline in-memory (Poppler can still emit debug output via debugger)
        rreq.uniqIdPrefix = utils::makeUniqId();
        rreq.filePrefix = "poppler_render";
        rreq.cancelFlag = req.cancelFlag;

        core::errors::report({
            core::errors::ErrorSeverity::Info,
            "core::import::DefaultImportStatementStrategy::run",
            std::string("poppler render start: ") + rreq.pdfPath.string() + " dpi=" + std::to_string(rreq.dpi),
            {}
        });
        report(0.05, "Rendering pages");
        const auto tRender0 = std::chrono::steady_clock::now();
        auto renderRes = poppler_->render(rreq);
        renderSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tRender0).count();
        report(0.15, "Rendered pages");

        if (req.cancelFlag && req.cancelFlag->load()) { report(0.0, "Canceled"); return out; }

        api::poppler::ExtractRequest ereq;
        ereq.pdfPath = rreq.pdfPath;
        ereq.dpi = rreq.dpi;
        ereq.outputDir = std::filesystem::path(); // keep production pipeline in-memory
        ereq.uniqIdPrefix = utils::makeUniqId();
        ereq.filePrefix = "poppler_extract";
        ereq.cancelFlag = req.cancelFlag;
        report(0.16, "Extracting text");
        const auto tExtract0 = std::chrono::steady_clock::now();
        auto extractRes = poppler_->extract(ereq);
        extractSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tExtract0).count();
        report(0.22, "Extracted text");

        std::string carriedBookingDate;
        int nextTxIndex = 1;

        auto readBytes = [](const std::filesystem::path& p) -> std::vector<uint8_t> {
            std::vector<uint8_t> out;
            try {
                std::ifstream ifs(p, std::ios::binary);
                if (!ifs) return out;
                out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::readBytes", std::current_exception()); }
            return out;
        };

        core::jobs::Scheduler localScheduler(4, 128);
        core::jobs::SlotLimiter localOcrLimiter(2);
        core::jobs::Scheduler* sched = req.scheduler ? req.scheduler : &localScheduler;
        core::jobs::SlotLimiter* ocrLimiter = req.ocrLimiter ? req.ocrLimiter : &localOcrLimiter;

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

        const size_t totalPages = std::max(renderRes.images.size(), renderRes.imageBytes.size());
        std::vector<std::future<PageWork>> pageFutures;
        pageFutures.reserve(totalPages);

        static constexpr size_t unitsPerPage = 4;
        const size_t totalUnits = std::max<size_t>(1, totalPages * unitsPerPage);
        std::atomic<size_t> doneUnits{0};
        std::mutex artifactsMutex;

        std::vector<PageWork> pages;
        pages.resize(totalPages);

        for (size_t pi = 0; pi < totalPages; ++pi) {
            auto prom = std::make_shared<std::promise<PageWork>>();
            pageFutures.push_back(prom->get_future());

            sched->enqueue([&, pi, prom]() mutable {
                const auto tPage0 = std::chrono::steady_clock::now();
                PageWork pw;
                pw.pageIndex = pi;

                size_t localUnitsDone = 0;
                auto unitDone = [&](size_t inc, const std::string& label) {
                    localUnitsDone += inc;
                    const size_t d = doneUnits.fetch_add(inc) + inc;
                    const double frac = std::min(1.0, static_cast<double>(d) / static_cast<double>(totalUnits));
                    const double pr = 0.22 + 0.60 * frac;
                    report(pr, std::string("[") + std::to_string(pi + 1) + "/" + std::to_string(totalPages) + "] " + label);
                };
                auto finishUnits = [&](const std::string& label) {
                    if (localUnitsDone >= unitsPerPage) return;
                    unitDone(unitsPerPage - localUnitsDone, label);
                };

                if (req.cancelFlag && req.cancelFlag->load()) {
                    finishUnits("Canceled");
                    prom->set_value(std::move(pw));
                    return;
                }

                std::vector<uint8_t> pageBytes;
                if (pi < renderRes.imageBytes.size() && !renderRes.imageBytes[pi].empty()) {
                    pageBytes = renderRes.imageBytes[pi];
                } else if (pi < renderRes.images.size() && !renderRes.images[pi].empty()) {
                    pageBytes = readBytes(renderRes.images[pi]);
                }
                if (pageBytes.empty()) {
                    finishUnits("No image");
                    prom->set_value(std::move(pw));
                    return;
                }

                api::opencv::MaskRequest mreq;
                mreq.imageBytes = pageBytes;
                mreq.uniqIdPrefix = utils::makeUniqId();
                mreq.filePrefix = "opencv_mask_page" + std::to_string(pi + 1);
                mreq.usePoppler = true;
                mreq.useMorphology = true;
                mreq.useTesseract = false;
                mreq.cancelFlag = req.cancelFlag;

                if (pi < extractRes.pages.size()) {
                    const auto& page = extractRes.pages[pi];
                    double scaleX = page.dpiX / 72.0;
                    double scaleY = page.dpiY / 72.0;
                    for (const auto& te : page.textElements) {
                        api::opencv::Rect r;
                        r.x = static_cast<int>(std::round(te.x * scaleX));
                        r.y = static_cast<int>(std::round(te.y * scaleY));
                        r.width = static_cast<int>(std::round(te.width * scaleX));
                        r.height = static_cast<int>(std::round(te.height * scaleY));
                        if (r.width <= 1 || r.height <= 1) continue;
                        mreq.textElements.push_back(r);
                    }
                }

                if (mreq.textElements.empty()) {
                    mreq.useTesseract = true;
                }

                if (mreq.useTesseract) {
                    try {
                        api::tesseract::ExtractRequest treq;
                        treq.imageBytes = pageBytes;
                        treq.tessdataPath = "";
                        treq.psm = 3;
                        treq.cancelFlag = req.cancelFlag;

                        if (ocrLimiter) ocrLimiter->acquire();
                        const auto tOcr0 = std::chrono::steady_clock::now();
                        auto tres = tesseract_->extract(treq);
                        pw.ocrSec += std::chrono::duration<double>(std::chrono::steady_clock::now() - tOcr0).count();
                        if (ocrLimiter) ocrLimiter->release();

                        mreq.tesseractTsv = tres.tsv;
                    } catch (...) {
                        try { if (ocrLimiter) ocrLimiter->release(); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseMask", std::current_exception()); }
                        core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractMaskExtract", std::current_exception());
                    }
                }

                auto maskRes = opencv_->mask(mreq);
                std::vector<uint8_t> maskedBytes = !maskRes.maskedImageBytes.empty() ? maskRes.maskedImageBytes : pageBytes;
                unitDone(1, "Mask");

                api::opencv::DetectRequest dreq;
                dreq.imageBytes = maskedBytes;
                dreq.uniqIdPrefix = utils::makeUniqId();
                dreq.filePrefix = "opencv_detect_tables_page" + std::to_string(pi + 1);
                dreq.kind = api::opencv::DetectRequest::DetectKind::Tables;
                dreq.cancelFlag = req.cancelFlag;
                auto detectRes = opencv_->detect(dreq);
                unitDone(1, "Detect");

                if (!detectRes.detected) {
                    finishUnits("No table");
                    prom->set_value(std::move(pw));
                    return;
                }

                api::opencv::CropRequest cropReq;
                cropReq.imageBytes = pageBytes;
                cropReq.uniqIdPrefix = utils::makeUniqId();
                cropReq.filePrefix = "opencv_crop_table_page" + std::to_string(pi + 1);
                cropReq.bbox = detectRes.table.bbox;
                cropReq.cancelFlag = req.cancelFlag;
                auto cropRes = opencv_->crop(cropReq);
                unitDone(1, "Crop");

                if (cropRes.croppedImageBytes.empty() || cropRes.croppedImageBytes.front().empty()) {
                    finishUnits("No crop");
                    prom->set_value(std::move(pw));
                    return;
                }

                api::tesseract::ExtractRequest oreq;
                oreq.kind = api::tesseract::ExtractRequest::Kind::Table;
                oreq.imageBytes = cropRes.croppedImageBytes.front();
                oreq.uniqIdPrefix = utils::makeUniqId();
                oreq.filePrefix = "tesseract_extract_table_page" + std::to_string(pi + 1);
                oreq.tessdataPath = "";
                oreq.psm = 6;
                oreq.cancelFlag = req.cancelFlag;

                oreq.cells.reserve(detectRes.table.cells.size());
                for (const auto& c : detectRes.table.cells) {
                    api::tesseract::Cell tc;
                    tc.row = c.row;
                    tc.col = c.col;
                    tc.bbox.x = c.bbox.x - detectRes.table.bbox.x;
                    tc.bbox.y = c.bbox.y - detectRes.table.bbox.y;
                    tc.bbox.width = c.bbox.width;
                    tc.bbox.height = c.bbox.height;
                    oreq.cells.push_back(tc);
                }

                try {
                    if (ocrLimiter) ocrLimiter->acquire();
                    const auto tOcr0 = std::chrono::steady_clock::now();
                    pw.ocr = tesseract_->extract(oreq);
                    pw.ocrSec += std::chrono::duration<double>(std::chrono::steady_clock::now() - tOcr0).count();
                    if (ocrLimiter) ocrLimiter->release();
                } catch (...) {
                    try { if (ocrLimiter) ocrLimiter->release(); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseTable", std::current_exception()); }
                    core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractTableExtract", std::current_exception());
                    finishUnits("OCR failed");
                    prom->set_value(std::move(pw));
                    return;
                }

                unitDone(1, "OCR");

                pw.hasTable = true;
                pw.table = detectRes.table;
                pw.cropBytes = std::move(cropRes.croppedImageBytes.front());
                pw.ocrWords = pw.ocr.words.size();

                try {
                    std::string tsvKey = "tesseract/page_" + std::to_string(pi) + "_crop_0.tsv";
                    std::vector<uint8_t> tsvData(pw.ocr.tsv.begin(), pw.ocr.tsv.end());
                    std::lock_guard<std::mutex> g(artifactsMutex);
                    out.artifacts[tsvKey] = std::move(tsvData);
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::storeTsvArtifact", std::current_exception()); }

                finishUnits("Done");
                pw.totalSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tPage0).count();
                prom->set_value(std::move(pw));
            });
        }

        for (auto& f : pageFutures) {
            try {
                auto pw = f.get();
                if (pw.pageIndex < pages.size()) pages[pw.pageIndex] = std::move(pw);
            } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::pageFutureGet", std::current_exception()); }
        }

        std::filesystem::path proofDir;
        if (debugger_ && debugger_->enabled()) {
            proofDir = runRoot / "proof";
            try { std::filesystem::create_directories(proofDir); } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::createProofDir", std::current_exception()); }
        }

        const auto tFinalize0 = std::chrono::steady_clock::now();
        size_t pagesWithTable = 0;
        size_t totalOcrWords = 0;
        double maxPageSec = 0.0;
        double sumPageSec = 0.0;
        double sumOcrSec = 0.0;

        const size_t finalizePages = std::max<size_t>(1, pages.size());
        for (size_t pi = 0; pi < pages.size(); ++pi) {
            if (req.cancelFlag && req.cancelFlag->load()) { report(0.0, "Canceled"); break; }
            auto& pw = pages[pi];
            if (!pw.hasTable) continue;

            {
                const double frac = static_cast<double>(pi + 1) / static_cast<double>(finalizePages);
                const double pr = 0.82 + 0.18 * frac;
                report(pr, std::string("[") + std::to_string(pi + 1) + "/" + std::to_string(finalizePages) + "] Finalize");
            }

            ++pagesWithTable;
            totalOcrWords += pw.ocrWords;
            maxPageSec = std::max(maxPageSec, pw.totalSec);
            sumPageSec += pw.totalSec;
            sumOcrSec += pw.ocrSec;

            auto parsed = DefaultStatementParser::parse(pw.table,
                                                      pw.ocr,
                                                      std::string(),
                                                      opencv_,
                                                      pw.cropBytes,
                                                      proofDir,
                                                      carriedBookingDate,
                                                      nextTxIndex);
            carriedBookingDate = parsed.lastBookingDate;
            nextTxIndex = parsed.nextTransactionIndex;

            if (!parsed.transactions.empty()) {
                for (auto& txptr : parsed.transactions) all.push_back(std::move(txptr));
            }

            if (!parsed.artifacts.empty()) {
                try {
                    std::lock_guard<std::mutex> g(artifactsMutex);
                    for (auto& kv : parsed.artifacts) out.artifacts.emplace(kv.first, std::move(kv.second));
                } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::mergeParsedArtifacts", std::current_exception()); }
            }

        finalizeSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tFinalize0).count();
        }

        if (!all.empty()) {
            stmt.transactions = std::move(all);
            out.data = std::make_shared<Statement>(std::move(stmt));
        }

        try {
            nlohmann::json j;
            j["jobId"] = req.jobId;
            j["sourcePath"] = req.sourcePath;
            j["pagesTotal"] = totalPages;
            j["pagesWithTable"] = pagesWithTable;
            j["renderSec"] = renderSec;
            j["extractSec"] = extractSec;
            j["finalizeSec"] = finalizeSec;
            j["pageWorkSecMax"] = maxPageSec;
            j["pageWorkSecSum"] = sumPageSec;
            j["ocrSecSum"] = sumOcrSec;
            j["ocrWordsTotal"] = totalOcrWords;
            j["totalSec"] = std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count();

            auto& arr = j["pages"];
            arr = nlohmann::json::array();
            for (const auto& pw : pages) {
                nlohmann::json pj;
                pj["index"] = pw.pageIndex;
                pj["hasTable"] = pw.hasTable;
                pj["totalSec"] = pw.totalSec;
                pj["ocrSec"] = pw.ocrSec;
                pj["ocrWords"] = pw.ocrWords;
                arr.push_back(std::move(pj));
            }

            const auto dumped = j.dump(2);
            out.artifacts["metrics.json"] = std::vector<uint8_t>(dumped.begin(), dumped.end());
        } catch (...) { core::errors::reportException(core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::metricsArtifact", std::current_exception()); }

        report(1.0, "Done");
        return out;
    }

private:
    std::shared_ptr<api::poppler::IPopplerService> poppler_;
    std::shared_ptr<api::opencv::IOpenCvService> opencv_;
    std::shared_ptr<api::tesseract::ITesseractService> tesseract_;
    std::shared_ptr<IDebugger> debugger_;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
    std::shared_ptr<api::opencv::IOpenCvService> opencv,
    std::shared_ptr<api::tesseract::ITesseractService> tesseract,
    std::shared_ptr<IDebugger> debugger) {
    return std::make_unique<DefaultImportStatementStrategy>(std::move(poppler), std::move(opencv), std::move(tesseract), std::move(debugger));
}
