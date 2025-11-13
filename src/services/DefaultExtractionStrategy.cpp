#include "pch.h"
#include "services/IExtractionStrategy.h"
#include "services/poppler/IPopplerService.h"
#include "services/opencv/IOpenCvService.h"
#include "services/tesseract/ITesseractService.h"
#include "models/StatementData.h"
#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>

// Default strategy: matches current controller pipeline (Poppler -> OpenCV -> OCR (concat only) -> StatementData)
class DefaultExtractionStrategyImpl : public IExtractionStrategy {
public:
	DefaultExtractionStrategyImpl(std::shared_ptr<IPopplerService> poppler,
		std::shared_ptr<IOpenCvService> layout,
		std::shared_ptr<ITesseractService> tesseract)
		: poppler_(std::move(poppler)), layout_(std::move(layout)), ocr_(std::move(tesseract)) {
	}

	StatementExtractionResult run(const StatementExtractionRequest& req) override {
		StatementExtractionResult out;
		if (!poppler_ || !layout_ || !ocr_) return out;

		// 1) extract metadata (text runs, bboxes)
		ExtractRequest ereq;
		ereq.pdfPath = std::filesystem::path(req.sourcePath);
		ereq.dpi = 300.0;
		auto extractRes = poppler_->extract(ereq);
		// parse pages array from JSON
		nlohmann::json pagesJson = nlohmann::json::array();
		try {
			if (!extractRes.metadata.is_null() && extractRes.metadata.contains("pages")) pagesJson = extractRes.metadata["pages"];
		} catch (...) { pagesJson = nlohmann::json::array(); }

		// 2) render images
		RenderRequest rreq;
		rreq.pdfPath = std::filesystem::path(req.sourcePath);
		rreq.dpi = 300.0;
		auto renderRes = poppler_->render(rreq);

		if (pagesJson.empty() && renderRes.images.empty()) return out;

		// 3) Build OpenCV layout requests by combining metadata (textElements) and image paths
		LayoutBatchRequest lreq;
		size_t pageCount = std::max<size_t>(pagesJson.size(), renderRes.images.size());
		lreq.pages.reserve(pageCount);
		for (size_t i = 0; i < pageCount; ++i) {
			LayoutRequest lr;
			// set image path if available
			if (i < renderRes.images.size()) lr.imagePath = renderRes.images[i].string();

			// metadata: read page-level json if available
			if (i < pagesJson.size() && pagesJson[i].is_object()) {
				auto &pj = pagesJson[i];
				// dpi/page metrics
				double dpiX = pj.value("dpi_x", (double)rreq.dpi);
				double dpiY = pj.value("dpi_y", (double)rreq.dpi);
				double pageWidthPts = pj.value("page_width_pts", 0.0);
				double pageHeightPts = pj.value("page_height_pts", 0.0);
				lr.dpiX = dpiX; lr.dpiY = dpiY; lr.pageWidthPts = pageWidthPts; lr.pageHeightPts = pageHeightPts;
				double sx = dpiX / 72.0; double sy = dpiY / 72.0;
				if (pj.contains("text_elements") && pj["text_elements"].is_array()) {
					for (const auto &te : pj["text_elements"]) {
						LayoutTextElement lte;
						lte.text = te.value("text", std::string());
						if (te.contains("bbox_pts") && te["bbox_pts"].is_array() && te["bbox_pts"].size() >= 4) {
							double x = te["bbox_pts"][0].get<double>();
							double y = te["bbox_pts"][1].get<double>();
							double w = te["bbox_pts"][2].get<double>();
							double h = te["bbox_pts"][3].get<double>();
							lte.x = x * sx; lte.y = y * sy; lte.width = w * sx; lte.height = h * sy;
						}
						lr.textElements.push_back(std::move(lte));
					}
				}
			}
			lreq.pages.push_back(std::move(lr));
		}

		StatementRequest sreq; sreq.sourceFile = req.sourcePath; sreq.pages = std::move(lreq.pages); sreq.requestId = "";
		auto sres = layout_->processStatement(sreq);
		auto layRes = sres.layoutBatchResult;

		// 4) build OCR request (concat only)
		OcrBatchRequest ocrReq; ocrReq.tessdataPath = "C:/coding/fossredder/res/tessdata/";
		ocrReq.inputsPerPage.reserve(layRes.pageOutputs.size());
		for (const auto& outpage : layRes.pageOutputs) {
			std::vector<OcrRequest> pageInputs;
			auto itConcat = std::find_if(outpage.generatedFileBlobs.begin(), outpage.generatedFileBlobs.end(), [](const auto& p) { return p.first.find("_tables_concat") != std::string::npos; });
			if (itConcat != outpage.generatedFileBlobs.end()) {
				OcrRequest orq; orq.data = itConcat->second; orq.tableBbox = RectDto(); orq.tessdataPath = ocrReq.tessdataPath; orq.psm = 3;
				pageInputs.push_back(std::move(orq));
			}
			ocrReq.inputsPerPage.push_back(std::move(pageInputs));
		}
		auto ocrRes = ocr_->extractTablesBatch(ocrReq);

		// 5) build StatementData
		out.data = std::make_shared<StatementData>(req.sourcePath, std::vector<uint8_t>(), renderRes, layRes.pageOutputs, ocrRes.outputsPerPage);
		return out;
	}

private:
	std::shared_ptr<IPopplerService> poppler_;
	std::shared_ptr<IOpenCvService> layout_;
	std::shared_ptr<ITesseractService> ocr_;
};

std::unique_ptr<IExtractionStrategy> createDefaultExtractionStrategy(std::shared_ptr<IPopplerService> poppler,
	std::shared_ptr<IOpenCvService> layout,
	std::shared_ptr<ITesseractService> tesseract) {
	return std::make_unique<DefaultExtractionStrategyImpl>(std::move(poppler), std::move(layout), std::move(tesseract));
}
