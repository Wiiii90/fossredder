#pragma once

#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResult.h"
#include <memory>
#include <vector>

namespace api::tesseract {

class ITesseractAdapter {
public:
    virtual ~ITesseractAdapter() = default;
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

}
