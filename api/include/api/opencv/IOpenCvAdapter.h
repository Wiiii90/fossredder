#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResult.h"
#include <memory>

namespace api::opencv {

class IOpenCvAdapter {
public:
    virtual ~IOpenCvAdapter() = default;

    virtual DenoiseResult denoise(const DenoiseRequest& req) = 0;
    virtual MaskResult mask(const MaskRequest& req) = 0;
    virtual DetectResult detect(const DetectRequest& req) = 0;
    virtual CropResult crop(const CropRequest& req) = 0;
};

}
