#pragma once

#include "TesseractDTO.h"
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class TesseractEngine {
public:
    // Recognize full text
    static OcrTextDto recognizeText(const OcrRequest& input, std::shared_ptr<IDebugger> debugger = nullptr);

    // Extract word-level information (TSV parsing)
    static OcrWordsDto getWords(const OcrRequest& input, std::shared_ptr<IDebugger> debugger = nullptr);

    // Extract table (cells + words) from request
    static OcrTableDto extractTable(const OcrRequest& input, std::shared_ptr<IDebugger> debugger = nullptr);
};
