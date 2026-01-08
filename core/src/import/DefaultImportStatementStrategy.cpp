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
#include "core/parser/DefaultStatementParser.h"
#include "../../debug/include/debug/IDebugger.h"
#include "core/models/Statement.h"
#include "core/utils/UniqId.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>
#include <limits>

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

        const std::filesystem::path runRoot(req.runRoot);
        try { std::filesystem::create_directories(runRoot); } catch (...) {}

        auto report = [&](double p, const std::string& phase) {
            if (req.progressCallback) {
                try { req.progressCallback(p, phase); } catch (...) {}
            }
        };

        report(0.02, "Preparing import");

        Statement stmt;
        std::vector<Transaction> all;

        api::poppler::RenderRequest rreq;
        rreq.pdfPath = std::filesystem::path(req.sourcePath);
        rreq.dpi = 300.0;
        rreq.outputDir = runRoot;
        rreq.uniqIdPrefix = utils::makeUniqId();
        rreq.filePrefix = "poppler_render";

        try { std::clog << "DefaultImportStatementStrategy: poppler render start: " << rreq.pdfPath.string() << " dpi=" << rreq.dpi << std::endl; } catch(...){ }
        report(0.05, "Rendering pages");
        auto renderRes = poppler_->render(rreq);
        report(0.15, "Rendered pages");

        api::poppler::ExtractRequest ereq;
        ereq.pdfPath = rreq.pdfPath;
        ereq.dpi = rreq.dpi;
        ereq.outputDir = runRoot;
        ereq.uniqIdPrefix = utils::makeUniqId();
        ereq.filePrefix = "poppler_extract";
        report(0.16, "Extracting text");
        auto extractRes = poppler_->extract(ereq);
        report(0.22, "Extracted text");

        std::string carriedBookingDate;
        int nextTxIndex = 1;

        size_t totalPages = renderRes.images.size();
        for (size_t pi = 0; pi < totalPages; ++pi) {
            const auto& pageImage = renderRes.images[pi];

            double pageBase = 0.22;
            if (totalPages > 0) pageBase += (0.6 * (static_cast<double>(pi) / static_cast<double>(totalPages)));
            report(pageBase, std::string("Processing page ") + std::to_string(pi + 1));

            try { std::clog << "DefaultImportStatementStrategy: opencv mask start page=" << pi << " image=" << pageImage.string() << std::endl; } catch(...){ }
            api::opencv::MaskRequest mreq;
            mreq.imagePath = pageImage;
            mreq.outputDir = runRoot;
            mreq.uniqIdPrefix = utils::makeUniqId();
            mreq.filePrefix = "opencv_mask_page" + std::to_string(pi + 1);
            mreq.usePoppler = true;
            mreq.useMorphology = true;
            mreq.useTesseract = false;

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

            try {
                api::opencv::DetectRequest tdreq;
                tdreq.imagePath = pageImage;
                tdreq.outputDir = runRoot;
                tdreq.uniqIdPrefix = utils::makeUniqId();
                tdreq.filePrefix = "opencv_detect_textblocks_page" + std::to_string(pi + 1);
                tdreq.kind = api::opencv::DetectRequest::DetectKind::TextBlocks;
                auto tdres = opencv_->detect(tdreq);
                if (tdres.detected) {
                    auto intersectsArea = [](const api::opencv::Rect &a, const api::opencv::Rect &b) -> int {
                        int x0 = std::max(a.x, b.x);
                        int y0 = std::max(a.y, b.y);
                        int x1 = std::min(a.x + a.width, b.x + b.width);
                        int y1 = std::min(a.y + a.height, b.y + b.height);
                        if (x1 <= x0 || y1 <= y0) return 0;
                        return (x1 - x0) * (y1 - y0);
                    };

                    for (const auto &tb : tdres.textBlocks) {
                        bool dup = false;
                        int areaTb = std::max(1, tb.width * tb.height);
                        for (const auto &existing : mreq.textElements) {
                            int inter = intersectsArea(tb, existing);
                            int minA = std::min(areaTb, std::max(1, existing.width * existing.height));
                            if (inter > (minA / 2)) { dup = true; break; }
                        }
                        if (!dup) mreq.textElements.push_back(tb);
                    }
                }
            } catch (...) {}

            if (mreq.textElements.empty()) {
                mreq.useTesseract = true;
            }

            if (mreq.useTesseract) {
                try {
                    api::tesseract::ExtractRequest treq;
                    treq.imagePath = pageImage;
                    treq.tessdataPath = "";
                    treq.psm = 3;
                    auto tres = tesseract_->extract(treq);
                    mreq.tesseractTsv = tres.tsv;
                } catch (...) {}
            }

            auto maskRes = opencv_->mask(mreq);
            std::filesystem::path maskedImage = maskRes.maskedImagePath.empty() ? pageImage : maskRes.maskedImagePath;
            try { std::clog << "DefaultImportStatementStrategy: opencv mask done page=" << pi << " maskedImage=" << maskedImage.string() << std::endl; } catch(...){ }

            try { std::clog << "DefaultImportStatementStrategy: opencv detect start page=" << pi << " image=" << maskedImage.string() << std::endl; } catch(...){ }
            api::opencv::DetectRequest dreq;
            dreq.imagePath = maskedImage;
            dreq.outputDir = runRoot;
            dreq.uniqIdPrefix = utils::makeUniqId();
            dreq.filePrefix = "opencv_detect_tables_page" + std::to_string(pi + 1);
            dreq.kind = api::opencv::DetectRequest::DetectKind::Tables;
            auto detectRes = opencv_->detect(dreq);
            try { std::clog << "DefaultImportStatementStrategy: opencv detect done page=" << pi << " detected=" << (detectRes.detected ? 1 : 0) << std::endl; } catch(...){ }

            if (!detectRes.detected) {
                try { std::clog << "DefaultImportStatementStrategy: no table detected on page=" << pi << ", skipping crop and OCR." << std::endl; } catch(...){ }
                continue;
            }

            // Crop/OCR must use the original page image. The masked image is for detection only.
            const std::filesystem::path cropSourceImage = pageImage;

            try { std::clog << "DefaultImportStatementStrategy: opencv crop start page=" << pi << " image=" << cropSourceImage.string() << std::endl; } catch(...){ }
            api::opencv::CropRequest cropReq;
            cropReq.imagePath = cropSourceImage;
            cropReq.outputDir = runRoot;
            cropReq.uniqIdPrefix = utils::makeUniqId();
            cropReq.filePrefix = "opencv_crop_table_page" + std::to_string(pi + 1);
            cropReq.bbox = detectRes.table.bbox;
            auto cropRes = opencv_->crop(cropReq);
            try { std::clog << "DefaultImportStatementStrategy: opencv crop done page=" << pi << " crops=" << cropRes.croppedImagePaths.size() << std::endl; } catch(...){ }

            for (size_t ci = 0; ci < cropRes.croppedImagePaths.size(); ++ci) {
                const auto& cropPath = cropRes.croppedImagePaths[ci];
                try { std::clog << "DefaultImportStatementStrategy: tesseract extractTable start page=" << pi << " crop=" << ci << " path=" << cropPath.string() << std::endl; } catch(...){ }

                api::tesseract::ExtractRequest oreq;
                oreq.kind = api::tesseract::ExtractRequest::Kind::Table;
                oreq.imagePath = cropPath;
                oreq.outputDir = runRoot;
                oreq.uniqIdPrefix = utils::makeUniqId();
                oreq.filePrefix = "tesseract_extract_table_page" + std::to_string(pi + 1);
                oreq.tessdataPath = "";
                oreq.psm = 3;

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

                auto ores = tesseract_->extract(oreq);
                try { std::clog << "DefaultImportStatementStrategy: tesseract extract done page=" << pi << " crop=" << ci << " words=" << ores.words.size() << " tsv_len=" << ores.tsv.size() << std::endl; } catch(...){ }

                auto parsed = DefaultStatementParser::parse(detectRes.table, ores, cropPath.string(), opencv_, carriedBookingDate, nextTxIndex);
                carriedBookingDate = parsed.lastBookingDate;
                nextTxIndex = parsed.nextTransactionIndex;

                if (debugger_ && debugger_->enabled()) {
                    try {
                        {
                            std::ostringstream dbg;
                            if (!parsed.debugLines.empty()) {
                                for (const auto& l : parsed.debugLines) dbg << l << "\n";
                            } else {
                                dbg << "(no debugLines)\n";
                            }
                            debugger_->writeText("DefaultStatementParser_debug.txt", dbg.str());
                        }
                        if (!parsed.transactions.empty()) {
                            std::ostringstream dbg;
                            dbg << "bookingDate\tvaluta\tamount\tname\tactorProposal\tmetadata\tproofImagePath\n";
                            for (const auto& tx : parsed.transactions) {
                                dbg << tx.bookingDate << "\t"
                                    << tx.valuta << "\t"
                                    << tx.amount << "\t"
                                    << tx.name << "\t"
                                    << tx.actorProposal << "\t"
                                    << tx.metadata << "\t"
                                    << tx.proofImagePath << "\n";
                            }
                            debugger_->writeText("DefaultStatementParser_transactions.txt", dbg.str());
                        }
                    } catch (...) {}
                }

                if (!parsed.transactions.empty()) {
                    for (auto& tx : parsed.transactions) all.push_back(std::move(tx));
                }

                std::string tsvKey = "tesseract/page_" + std::to_string(pi) + "_crop_" + std::to_string(ci) + ".tsv";
                std::vector<uint8_t> tsvData(ores.tsv.begin(), ores.tsv.end());
                out.artifacts[tsvKey] = std::move(tsvData);

                if (!ores.tables.empty()) {
                    std::ostringstream cellText;
                    cellText << "rows\t" << detectRes.table.rows << "\tcols\t" << detectRes.table.cols << "\n";
                    for (const auto& cell : ores.tables[0].cells) {
                        cellText << cell.row << "\t" << cell.col << "\t" << cell.confidence << "\t" << cell.text << "\n";
                    }
                    auto cellStr = cellText.str();
                    std::string cellsKey = "tesseract/page_" + std::to_string(pi) + "_crop_" + std::to_string(ci) + "_cells.tsv";
                    out.artifacts[cellsKey] = std::vector<uint8_t>(cellStr.begin(), cellStr.end());
                }

                // update progress per crop
                double cropProgressBase = 0.22 + 0.6 * (static_cast<double>(pi) / static_cast<double>(totalPages));
                double cropPart = (static_cast<double>(ci + 1) / static_cast<double>(cropRes.croppedImagePaths.size()));
                double overall = cropProgressBase + 0.6 * (1.0 / static_cast<double>(totalPages)) * cropPart * 0.5; // small weight
                report(overall, std::string("OCR page ") + std::to_string(pi + 1) + " crop " + std::to_string(ci + 1));
            }
        }

        if (!all.empty()) {
            stmt.transactions = std::move(all);
            out.data = std::make_shared<Statement>(std::move(stmt));
        }

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
