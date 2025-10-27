#pragma once

#include "controllers/IImportController.h"
#include <QObject>
#include <memory>
#include <string>
#include <vector>

class IOcrEngine;
class IPdfRenderer;
class ITextCleaner;
class PdfExtractedData;
class Transaction;

class PdfImportController : public QObject, public IImportController {
    Q_OBJECT
public:
    PdfImportController(
        std::shared_ptr<IOcrEngine> ocrEngine,
        std::shared_ptr<IPdfRenderer> pdfRenderer,
        std::shared_ptr<ITextCleaner> textCleaner, // optional ONNX cleaner
        QObject* parent = nullptr);

    std::shared_ptr<void> extractData(const std::string& filePath) override;
    std::shared_ptr<PdfExtractedData> extractPdfData(const std::string& filePath);

signals:
    void transactionsExtracted(const std::vector<std::shared_ptr<Transaction>>& transactions);

private:
    std::shared_ptr<IOcrEngine> ocrEngine_;
    std::shared_ptr<IPdfRenderer> pdfRenderer_;
    std::shared_ptr<ITextCleaner> textCleaner_; // may be null
};