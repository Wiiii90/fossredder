#include "pch.h"
#include "services/tesseract/ITesseractAdapter.h"
#include "services/tesseract/TesseractEngine.h"
#include "debug/IDebugger.h"

class TesseractAdapterImpl : public ITesseractAdapter {
public:
    explicit TesseractAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    OcrTextDto recognizeTextFromBytes(const OcrInputDto& input, const std::string& tessdataPath, int psm) override {
        OcrRequest req = input;
        req.tessdataPath = tessdataPath;
        req.psm = psm;
        return TesseractEngine::recognizeText(req, debugger);
    }

    OcrWordsDto getWordsFromBytes(const OcrInputDto& input, const std::string& tessdataPath, int psm, const std::string& debugPrefix) override {
        OcrRequest req = input;
        req.tessdataPath = tessdataPath;
        req.psm = psm;
        req.debugPrefix = debugPrefix;
        return TesseractEngine::getWords(req, debugger);
    }

    OcrTableDto extractTableFromBytes(const OcrRequest& input) override {
        return TesseractEngine::extractTable(input, debugger);
    }

private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<ITesseractAdapter> createTesseractAdapter() {
    return std::make_shared<TesseractAdapterImpl>();
}

std::shared_ptr<ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<TesseractAdapterImpl>(std::move(debugger));
}