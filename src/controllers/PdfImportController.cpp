#include "pch.h"
#include "controllers/PdfImportController.h"
#include "models/PdfExtractedData.h"
#include "models/Header.h"
#include "models/Block.h"
#include "models/Page.h"
#include "models/Transaction.h"
#include "models/BookingGroup.h"
#include "views/ConsoleView.h"
#include "poppler/IPdfRenderer.h"
#include "ocr/IOcrEngine.h"
#include "llama/LlamaEngine.h"
#include "controllers/LlamaController.h"
#include <QDebug>
#include <filesystem>

PdfImportController::PdfImportController(
	std::shared_ptr<IOcrEngine> ocrEngine,
	std::shared_ptr<IPdfRenderer> pdfRenderer,
	std::shared_ptr<LlamaEngine> llamaEngine,
	QObject* parent)
	: QObject(parent),
	ocrEngine_(std::move(ocrEngine)),
	pdfRenderer_(std::move(pdfRenderer)),
	llamaEngine_(std::move(llamaEngine))
{
}

std::shared_ptr<void> PdfImportController::extractData(const std::string& filePath) {
	return extractPdfData(filePath);
}

std::shared_ptr<PdfExtractedData> PdfImportController::extractPdfData(const std::string& filePath) {
	ConsoleView consoleView;
	if (!std::filesystem::exists(filePath)) {
		consoleView.displayError("PDF file does not exist: " + filePath);
		throw std::runtime_error("PDF file does not exist: " + filePath);
	}
	const std::string tessdataPath = "C:/coding/fossredder/res/tessdata/";
	const std::string outputPrefix = "page";

	std::vector<std::string> imageFiles = pdfRenderer_->renderToImages(filePath, outputPrefix);

	std::vector<std::shared_ptr<Page>> allPages;
	for (size_t i = 0; i < imageFiles.size(); ++i) {
		try {
			std::string xmlContent = ocrEngine_->recognizeAltoXml(imageFiles[i], tessdataPath);

			std::vector<std::string> headerKeywords = {
				"Angaben zu den Umsätzen", "Valuta", "zu Ihren Lasten", "zu Ihren Gunsten"
			};
			std::vector<std::string> footerKeywords = {
				"Folgeseite", "Neuer Kontostand", "Guthaben sind als Einlagen"
			};

			auto page = std::make_shared<Page>(xmlContent, static_cast<int>(i + 1), headerKeywords, footerKeywords);
			allPages.push_back(page);
		}
		catch (const std::exception& e) {
			consoleView.displayError(e.what());
		}
	}

	std::vector<BookingGroup> bookingGroups = BookingGroup::extractBookingGroups(allPages);

	std::vector<std::shared_ptr<Transaction>> allTransactions;
	for (auto& group : bookingGroups) {
		for (const auto& tx : group.getTransactions()) {
			allTransactions.push_back(std::make_shared<Transaction>(tx));
		}
	}

	qDebug() << "Vor enrichTransactions: Anzahl Transaktionen:" << allTransactions.size();
	for (size_t i = 0; i < allTransactions.size(); ++i) {
		if (!allTransactions[i]) qDebug() << "Transaktion" << i << "ist nullptr!";
	}

	if (llamaEngine_) {
		LlamaController::enrichTransactions(*llamaEngine_, allTransactions);
	}

	qDebug() << "Nach enrichTransactions: Anzahl Transaktionen:" << allTransactions.size();
	for (size_t i = 0; i < allTransactions.size(); ++i) {
		if (!allTransactions[i]) qDebug() << "Transaktion" << i << "ist nullptr!";
	}

	emit transactionsExtracted(allTransactions);
	auto pdfData = std::make_shared<PdfExtractedData>(filePath, allPages, bookingGroups);

	return pdfData;
}