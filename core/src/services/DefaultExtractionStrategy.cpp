#include "core/pch.h"
#include "core/services/IExtractionStrategy.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include "api/poppler/IPopplerService.h"
#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include "api/opencv/IOpenCvService.h"
#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResponse.h"
#include "api/tesseract/ITesseractService.h"
#include "core/services/IStatementExtractionService.h"
#include <filesystem>
#include <fstream>

class DefaultExtractionStrategyImpl : public IExtractionStrategy {
public:
    DefaultExtractionStrategyImpl(std::shared_ptr<api::poppler::IPopplerService> poppler,
        std::shared_ptr<api::opencv::IOpenCvService> opencv,
        std::shared_ptr<api::tesseract::ITesseractService> tesseract)
        : poppler_(std::move(poppler)), opencv_(std::move(opencv)), tesseract_(std::move(tesseract)) {
    }

    StatementExtractionResult run(const StatementExtractionRequest& req) override {
        StatementExtractionResult out;
        if (!poppler_ || !opencv_ || !tesseract_) return out;

        api::poppler::RenderRequest rreq;
        rreq.pdfPath = std::filesystem::path(req.sourcePath);
        rreq.dpi = 300.0;
        auto renderRes = poppler_->render(rreq);
        if (renderRes.images.empty()) return out;

        for (size_t pi = 0; pi < renderRes.images.size(); ++pi) {
            const auto& pageImage = renderRes.images[pi];

            api::opencv::MaskRequest mreq;
            mreq.imagePath = pageImage;
            auto maskRes = opencv_->mask(mreq);
            std::filesystem::path maskedImage = maskRes.maskedImagePath.empty() ? pageImage : maskRes.maskedImagePath;

            api::opencv::DetectRequest dreq;
            dreq.imagePath = maskedImage;
            dreq.kind = api::opencv::DetectRequest::DetectKind::Tables;
            auto detectRes = opencv_->detect(dreq);

            // crop whole page or detected regions; crop API currently returns vector of cropped image paths
            api::opencv::CropRequest cropReq;
            cropReq.imagePath = maskedImage;
            auto cropRes = opencv_->crop(cropReq);

            // If detect provided table bboxes we could map crops to tables; assume crop returns table crops in same order
            for (size_t ci = 0; ci < cropRes.croppedImagePaths.size(); ++ci) {
                const auto& cropPath = cropRes.croppedImagePaths[ci];
                api::tesseract::ExtractRequest oreq;
                oreq.imagePath = cropPath;
                oreq.tessdataPath = "";
                oreq.psm = 3;
                auto ores = tesseract_->extract(oreq);

                // save TSV artifact
                std::string tsvKey = "tesseract/page_" + std::to_string(pi) + "_crop_" + std::to_string(ci) + ".tsv";
                std::vector<uint8_t> tsvData(ores.tsv.begin(), ores.tsv.end());
                out.artifacts[tsvKey] = std::move(tsvData);

                // save text artifact
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

std::unique_ptr<IExtractionStrategy> createDefaultExtractionStrategy(std::shared_ptr<api::poppler::IPopplerService> poppler,
    std::shared_ptr<api::opencv::IOpenCvService> opencv,
    std::shared_ptr<api::tesseract::ITesseractService> tesseract) {
    return std::make_unique<DefaultExtractionStrategyImpl>(std::move(poppler), std::move(opencv), std::move(tesseract));
}
