#include "pch.h"
#include "services/tesseract/ITesseractService.h"
#include "services/tesseract/ITesseractAdapter.h"
#include "services/tesseract/TesseractDTO.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <utility>

class TesseractServiceImpl : public ITesseractService {
public:
    TesseractServiceImpl(std::shared_ptr<ITesseractAdapter> adapter) : adapter_(std::move(adapter)) {}

    OcrResult extractTable(const OcrRequest& req) override {
        OcrResult out;
        if (!adapter_) return out;
        out.tableResult = adapter_->extractTableFromBytes(req);
        out.wordsResult = adapter_->getWordsFromBytes(req, req.tessdataPath, req.psm, req.debugPrefix);
        out.textResult = adapter_->recognizeTextFromBytes(req, req.tessdataPath, req.psm);
        return out;
    }

    OcrBatchResult extractTablesBatch(const OcrBatchRequest& req) override {
        OcrBatchResult res;
        res.outputsPerPage.reserve(req.inputsPerPage.size());
        for (size_t pi = 0; pi < req.inputsPerPage.size(); ++pi) {
            const auto& pageInputs = req.inputsPerPage[pi];
            std::vector<OcrTableDto> pageOuts;
            pageOuts.reserve(pageInputs.size());
            for (size_t ti = 0; ti < pageInputs.size(); ++ti) {
                const auto& in = pageInputs[ti];
                try {
                    OcrTableDto out = adapter_->extractTableFromBytes(in);
                    pageOuts.push_back(std::move(out));
                }
                catch (...) {
                    pageOuts.push_back(OcrTableDto());
                }
            }
            res.outputsPerPage.push_back(std::move(pageOuts));
        }
        return res;
    }

private:
    std::shared_ptr<ITesseractAdapter> adapter_;
};

std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter) {
    return std::make_shared<TesseractServiceImpl>(std::move(adapter));
}
