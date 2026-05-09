#pragma once

#include "api/opencv/Types.h"
#include "core/ports/services/OpenCvResult.h"

namespace api::opencv {

using Rect = core::ports::services::opencv::Rect;
using Word = core::ports::services::opencv::Word;
using Cell = core::ports::services::opencv::Cell;
using Table = core::ports::services::opencv::Table;
using DenoiseResult = core::ports::services::opencv::DenoiseResult;
using MaskResult = core::ports::services::opencv::MaskResult;
using DetectResult = core::ports::services::opencv::DetectResult;
using CropResult = core::ports::services::opencv::CropResult;

} // namespace api::opencv
