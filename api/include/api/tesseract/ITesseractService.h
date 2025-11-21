#pragma once

#include "api/tesseract/TesseractRequest.h"
#include "api/tesseract/TesseractResponse.h"
#include <memory>

namespace api::tesseract {

class ITesseractService {
public:
    virtual ~ITesseractService() = default;

    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<class ITesseractAdapter> adapter);

}
