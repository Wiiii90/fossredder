#pragma once

#include "api/poppler/IPopplerAdapter.h"
#include "core/ports/services/IPopplerService.h"
#include <memory>

namespace api::poppler {

using IPopplerService = core::ports::services::IPopplerService;

std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter);

} // namespace api::poppler
