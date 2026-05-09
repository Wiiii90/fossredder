#pragma once

#include "api/opencv/Types.h"
#include "core/ports/services/OpenCvRequest.h"

namespace api::opencv {

using Rect = core::ports::services::opencv::Rect;
using DenoiseRequest = core::ports::services::opencv::DenoiseRequest;
using MaskRequest = core::ports::services::opencv::MaskRequest;
using DetectRequest = core::ports::services::opencv::DetectRequest;
using CropRequest = core::ports::services::opencv::CropRequest;

} // namespace api::opencv
