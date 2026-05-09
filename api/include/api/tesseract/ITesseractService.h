#pragma once

#include "api/tesseract/ITesseractAdapter.h"
#include "core/ports/services/ITesseractService.h"
#include <memory>

namespace api::tesseract {

using ITesseractService = core::ports::services::ITesseractService;

std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter);

} // namespace api::tesseract
