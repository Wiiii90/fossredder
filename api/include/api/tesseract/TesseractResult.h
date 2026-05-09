#pragma once

#include "api/tesseract/Types.h"
#include "core/ports/services/TesseractResult.h"

namespace api::tesseract {

using Rect = core::ports::services::tesseract::Rect;
using Text = core::ports::services::tesseract::Text;
using Word = core::ports::services::tesseract::Word;
using Cell = core::ports::services::tesseract::Cell;
using Table = core::ports::services::tesseract::Table;
using ExtractResult = core::ports::services::tesseract::ExtractResult;

} // namespace api::tesseract
