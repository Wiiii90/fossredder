#pragma once

#include "api/tesseract/Types.h"
#include "core/ports/services/TesseractRequest.h"

namespace api::tesseract {

using Rect = core::ports::services::tesseract::Rect;
using Text = core::ports::services::tesseract::Text;
using Word = core::ports::services::tesseract::Word;
using Cell = core::ports::services::tesseract::Cell;
using Table = core::ports::services::tesseract::Table;
using OcrEngineMode = core::ports::services::tesseract::OcrEngineMode;
using RecognitionSettings = core::ports::services::tesseract::RecognitionSettings;
using ExtractRequest = core::ports::services::tesseract::ExtractRequest;
inline constexpr const char* kDefaultStatementCharWhitelist = core::ports::services::tesseract::kDefaultStatementCharWhitelist;

} // namespace api::tesseract
