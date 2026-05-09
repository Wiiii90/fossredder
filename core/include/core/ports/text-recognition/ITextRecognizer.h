/**
 * @file core/include/core/ports/text-recognition/ITextRecognizer.h
 * @brief OCR and text recognition port.
 */
#pragma once

#include "core/ports/text-recognition/TesseractRequest.h"
#include "core/ports/text-recognition/TesseractResult.h"

namespace core::ports::text_recognition {

/**
 * @brief Recognizes text from images.
 */
class ITextRecognizer {
public:
    virtual ~ITextRecognizer() = default;

    /**
     * @brief Extracts text from an image or table region.
     * @param req OCR request.
     * @return The OCR result.
     */
    virtual tesseract::ExtractResult extract(const tesseract::ExtractRequest& req) = 0;
};

} // namespace core::ports::text_recognition
