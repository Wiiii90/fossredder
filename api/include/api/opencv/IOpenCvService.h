#pragma once

#include "api/opencv/IOpenCvAdapter.h"
#include "core/ports/services/IOpenCvService.h"
#include <memory>

namespace api::opencv {

using IOpenCvService = core::ports::services::IOpenCvService;

std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvAdapter> adapter);

} // namespace api::opencv
