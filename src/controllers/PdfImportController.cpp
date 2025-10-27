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
#include "onnx/ITextCleaner.h"
#include <QDebug>
#include <filesystem>
#include <codecvt>
#include <locale>

PdfImportController::PdfImportController(
	std::shared_ptr<IOcrEngine> ocrEngine,
	std::shared_ptr<IPdfRenderer> pdfRenderer,
	std::shared_ptr<ITextCleaner> textCleaner,
	QObject* parent)
	: QObject(parent),
	ocrEngine_(std::move(ocrEngine)),
	pdfRenderer_(std::move(pdfRenderer)),
	textCleaner_(std::move(textCleaner))
{
}

std::shared_ptr<void> PdfImportController::extractData(const std::string& filePath) {
	return extractPdfData(filePath);
}

// Hilfsfunktion: Entfernt ungültige UTF-8-Sequenzen
static std::string sanitize_utf8(const std::string& input) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    try {
        auto wstr = conv.from_bytes(input); // ungültige Sequenzen werden zu U+FFFD
        return conv.to_bytes(wstr);
    } catch (...) {
        // Fallback: Entferne alle nicht-ASCII-Zeichen
        std::string out;
        for (char c : input) if ((unsigned char)c < 128) out += c;
        return out;
    }
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
			xmlContent = sanitize_utf8(xmlContent); // <--- UTF-8-Säuberung

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

	// Apply cleaner directly to groups so ConsoleView shows cleaned details
	if (textCleaner_) {
		for (auto& group : bookingGroups) {
			group.transformTransactionDetails([&](const std::string& s){ return textCleaner_->clean(s); });
		}
	}

	std::vector<std::shared_ptr<Transaction>> allTransactions;
	for (auto& group : bookingGroups) {
		for (const auto& tx : group.getTransactions()) {
			allTransactions.push_back(std::make_shared<Transaction>(tx));
		}
	}

	// Optional: ONNX-based text cleaning applied here (redundant, keeps compatibility)
	qDebug() << "[ONNX] textCleaner_ is" << (textCleaner_ ? "set" : "NOT set");
	int txWithDetails = 0;
	for (const auto& tx : allTransactions) {
		if (tx && !tx->details.empty()) ++txWithDetails;
	}
	qDebug() << "[ONNX] Transactions with non-empty details:" << txWithDetails;
	if (textCleaner_) {
		for (auto& tx : allTransactions) {
			if (tx && !tx->details.empty()) {
				try {
					qDebug() << "[ONNX] Before cleaning:" << QString::fromStdString(tx->details);
					tx->details = textCleaner_->clean(tx->details);
					qDebug() << "[ONNX] After cleaning:" << QString::fromStdString(tx->details);
				} catch (const std::exception& e) {
					qDebug() << "ONNX clean failed:" << e.what();
				}
			}
		}
	}

	// PDF-Daten-Objekt erstellen
	auto pdfData = std::make_shared<PdfExtractedData>(filePath, allPages, bookingGroups);

	// Extrahierte Daten vor evtl. weiterer Anreicherung anzeigen
	consoleView.displayPdfData(pdfData);

	emit transactionsExtracted(allTransactions);

	return pdfData;
}