#pragma once

#include "api/tesseract/Types.h"
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class TesseractEngine {
public:
    static std::pair<api::tesseract::Text, std::vector<api::tesseract::Word>> extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> debugger = nullptr);
    static std::pair<api::tesseract::Text, std::vector<api::tesseract::Word>> extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, int psm, std::shared_ptr<IDebugger> debugger);
};
