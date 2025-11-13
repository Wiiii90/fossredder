#pragma once

#include "TesseractDTO.h"
#include <string>
#include <memory>
#include <vector>

class ITesseractAdapter;

class ITesseractService {
public:
    virtual ~ITesseractService() = default;

    virtual OcrResult extractTable(const OcrRequest& req) = 0;
    virtual OcrBatchResult extractTablesBatch(const OcrBatchRequest& req) = 0;
};

// Factory forward declarations - use Tesseract-named API only
std::shared_ptr<ITesseractService> createTesseractService(std::shared_ptr<ITesseractAdapter> adapter);
