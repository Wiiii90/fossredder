#pragma once

#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/Types.h"
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class TesseractEngine {
public:
    static std::pair<api::tesseract::Text, std::vector<api::tesseract::Word>> extractFromBytes(
        const std::vector<uint8_t>& data,
        const std::string& tessdataPath,
        const api::tesseract::RecognitionSettings& recognition,
        std::shared_ptr<IDebugger> debugger = nullptr);
};
