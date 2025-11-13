#pragma once

#include "TesseractDTO.h"
#include <string>
#include <memory>

class IDebugger;

class ITesseractAdapter {
public:
    virtual ~ITesseractAdapter() = default;

    virtual OcrTextDto recognizeTextFromBytes(const OcrInputDto& input, const std::string& tessdataPath, int psm) = 0;
    virtual OcrWordsDto getWordsFromBytes(const OcrInputDto& input, const std::string& tessdataPath, int psm, const std::string& debugPrefix) = 0;
    virtual OcrTableDto extractTableFromBytes(const OcrRequest& request) = 0;
};

std::shared_ptr<ITesseractAdapter> createTesseractAdapter();
std::shared_ptr<ITesseractAdapter> createTesseractAdapter(std::shared_ptr<IDebugger> debugger);
