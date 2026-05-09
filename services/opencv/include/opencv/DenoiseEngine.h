#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResult.h"
#include <memory>

class IDebugger;

namespace opencv {

class DenoiseEngine {
public:
    static api::opencv::DenoiseResult Denoise(const api::opencv::DenoiseRequest& req, std::shared_ptr<IDebugger> debugger = nullptr);
};

} // namespace opencv
