#pragma once

#include "tesseract/TesseractDTO.h"
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class TesseractEngine {
public:
    static std::pair<Text, std::vector<Word>> extractFromBytes(const std::vector<uint8_t>& data, const std::string& tessdataPath, std::shared_ptr<IDebugger> debugger = nullptr);
};
