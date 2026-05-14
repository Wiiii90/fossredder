/**
 * @file core/include/core/ports/text-recognition/ITextRecognizer.h
 * @brief Text recognition port.
 */
#pragma once

#include "core/ports/text-recognition/TextRecognitionRequest.h"
#include "core/ports/text-recognition/TextRecognitionResult.h"

namespace core::ports::text_recognition {

class ITextRecognizer {
public:
    virtual ~ITextRecognizer() = default;
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

} // namespace core::ports::text_recognition
