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
#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>
#include <limits>

class DefaultImportStatementStrategy : public IImportStatementStrategy {
public:
    DefaultImportStatementStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
        std::shared_ptr<api::opencv::IOpenCvService> opencv,
        std::shared_ptr<api::tesseract::ITesseractService> tesseract)
        : poppler_(std::move(poppler)), opencv_(std::move(opencv)), tesseract_(std::move(tesseract)) {
    }

    ImportResult run(const ImportRequest& req) override {
        ImportResult out;
        if (!poppler_ || !opencv_ || !tesseract_) return out;

        api::poppler::RenderRequest rreq;
        rreq.pdfPath = std::filesystem::path(req.sourcePath);
        rreq.dpi = 300.0;

        try { std::clog << "DefaultImportStatementStrategy: poppler render start: " << rreq.pdfPath.string() << " dpi=" << rreq.dpi << std::endl; } catch(...){}
        auto renderRes = poppler_->render(rreq);
        try { std::clog << "DefaultImportStatementStrategy: poppler render done, images=" << renderRes.images.size() << std::endl; } catch(...){}
        if (renderRes.images.empty()) return out;

        for (size_t pi = 0; pi < renderRes.images.size(); ++pi) {
            const auto& pageImage = renderRes.images[pi];

            try { std::clog << "DefaultImportStatementStrategy: opencv mask start page=" << pi << " image=" << pageImage.string() << std::endl; } catch(...){}
            api::opencv::MaskRequest mreq;
            mreq.imagePath = pageImage;
            auto maskRes = opencv_->mask(mreq);
            std::filesystem::path maskedImage = maskRes.maskedImagePath.empty() ? pageImage : maskRes.maskedImagePath;
            try { std::clog << "DefaultImportStatementStrategy: opencv mask done page=" << pi << " maskedImage=" << maskedImage.string() << std::endl; } catch(...){}

            try { std::clog << "DefaultImportStatementStrategy: opencv detect start page=" << pi << " image=" << maskedImage.string() << std::endl; } catch(...){}
            api::opencv::DetectRequest dreq;
            dreq.imagePath = maskedImage;
            dreq.kind = api::opencv::DetectRequest::DetectKind::Tables;
            auto detectRes = opencv_->detect(dreq);
            try { std::clog << "DefaultImportStatementStrategy: opencv detect done page=" << pi << " detected=" << (detectRes.detected ? 1 : 0) << std::endl; } catch(...){}

            // Only crop if a table was detected
            if (!detectRes.detected) {
                try { std::clog << "DefaultImportStatementStrategy: no table detected on page=" << pi << ", skipping crop and OCR." << std::endl; } catch(...){}
                continue;
            }

            try { std::clog << "DefaultImportStatementStrategy: opencv crop start page=" << pi << " image=" << maskedImage.string() << std::endl; } catch(...){}
            api::opencv::CropRequest cropReq;
            cropReq.imagePath = maskedImage;
            cropReq.bbox = detectRes.table.bbox;
            auto cropRes = opencv_->crop(cropReq);
            try { std::clog << "DefaultImportStatementStrategy: opencv crop done page=" << pi << " crops=" << cropRes.croppedImagePaths.size() << std::endl; } catch(...){}

            for (size_t ci = 0; ci < cropRes.croppedImagePaths.size(); ++ci) {
                const auto& cropPath = cropRes.croppedImagePaths[ci];
                try { std::clog << "DefaultImportStatementStrategy: tesseract extract start page=" << pi << " crop=" << ci << " path=" << cropPath.string() << std::endl; } catch(...){}
                api::tesseract::ExtractRequest oreq;
                oreq.imagePath = cropPath;
                oreq.tessdataPath = "";
                oreq.psm = 3;
                auto ores = tesseract_->extract(oreq);
                try { std::clog << "DefaultImportStatementStrategy: tesseract extract done page=" << pi << " crop=" << ci << " text_len=" << ores.text.size() << " tsv_len=" << ores.tsv.size() << std::endl; } catch(...){}

                std::string tsvKey = "tesseract/page_" + std::to_string(pi) + "_crop_" + std::to_string(ci) + ".tsv";
                std::vector<uint8_t> tsvData(ores.tsv.begin(), ores.tsv.end());
                out.artifacts[tsvKey] = std::move(tsvData);

                std::string txtKey = "tesseract/page_" + std::to_string(pi) + "_crop_" + std::to_string(ci) + ".txt";
                std::vector<uint8_t> txtData(ores.text.begin(), ores.text.end());
                out.artifacts[txtKey] = std::move(txtData);
            }
        }

        return out;
    }

private:
    std::shared_ptr<api::poppler::IPopplerService> poppler_;
    std::shared_ptr<api::opencv::IOpenCvService> opencv_;
    std::shared_ptr<api::tesseract::ITesseractService> tesseract_;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
    std::shared_ptr<api::opencv::IOpenCvService> opencv,
    std::shared_ptr<api::tesseract::ITesseractService> tesseract) {
    return std::make_unique<DefaultImportStatementStrategy>(std::move(poppler), std::move(opencv), std::move(tesseract));
}
