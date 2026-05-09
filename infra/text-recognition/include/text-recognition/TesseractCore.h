/**
 * @file infra/text-recognition/include/text-recognition/TesseractCore.h
 * @brief Declares the Tesseract implementation helpers used by the import workflow.
 */

#pragma once

#include "core/ports/text-recognition/TesseractRequest.h"
#include "core/ports/text-recognition/TesseractTypes.h"
#include <string>
#include <vector>
#include <memory>

class IDebugger;

class TesseractCore {
public:
    /**
     * @brief Extracts recognized text and word-level OCR data from image bytes.
     * @param data The encoded image bytes.
     * @param tessdataPath The tessdata directory path or a fallback location hint.
     * @param recognition The OCR recognition settings.
     * @param debugger Optional debugger used for trace output.
     * @return A pair containing recognized text and the extracted word list.
     */
    static std::pair<core::ports::text_recognition::tesseract::Text, std::vector<core::ports::text_recognition::tesseract::Word>> extractFromBytes(
        const std::vector<uint8_t>& data,
        const std::string& tessdataPath,
        const core::ports::text_recognition::tesseract::RecognitionSettings& recognition,
        std::shared_ptr<IDebugger> debugger = nullptr);
};
