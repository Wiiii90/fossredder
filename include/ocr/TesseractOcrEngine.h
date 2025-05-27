#pragma once
#include "ocr/IOcrEngine.h"
#include <string>

class TesseractOcrEngine : public IOcrEngine {
public:
    std::string recognizeAltoXml(const std::string& imageFilePath, const std::string& tessdataPath) override;
};