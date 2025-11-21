#pragma once

#include "api/opencv/IOpenCvAdapter.h"
#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include <memory>

namespace api::opencv {

class IOpenCvService {
public:
    virtual ~IOpenCvService() = default;

    virtual DenoiseResult denoise(const DenoiseRequest& req) const = 0;
    virtual MaskResult mask(const MaskRequest& req) const = 0;
    virtual DetectResult detect(const DetectRequest& req) const = 0;
    virtual CropResult crop(const CropRequest& req) const = 0;
};

std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<class IOpenCvAdapter> adapter);

}
