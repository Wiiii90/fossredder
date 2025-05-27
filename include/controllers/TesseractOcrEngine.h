#pragma once
#include "IOcrEngine.h"

class TesseractOcrEngine : public IOcrEngine {
public:
    std::string recognizeAltoXml(const std::string& imageFilePath, const std::string& tessdataPath) override;
};