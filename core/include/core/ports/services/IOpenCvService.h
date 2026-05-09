#pragma once

#include "core/ports/services/OpenCvRequest.h"
#include "core/ports/services/OpenCvResult.h"

namespace core::ports::services {

class IOpenCvService {
public:
    virtual ~IOpenCvService() = default;

    virtual opencv::DenoiseResult denoise(const opencv::DenoiseRequest& req) const = 0;
    virtual opencv::MaskResult mask(const opencv::MaskRequest& req) const = 0;
    virtual opencv::DetectResult detect(const opencv::DetectRequest& req) const = 0;
    virtual opencv::CropResult crop(const opencv::CropRequest& req) const = 0;
};

} // namespace core::ports::services
