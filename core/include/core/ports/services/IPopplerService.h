#pragma once

#include "core/ports/services/PopplerRequest.h"
#include "core/ports/services/PopplerResult.h"

namespace core::ports::services {

class IPopplerService {
public:
    virtual ~IPopplerService() = default;

    virtual poppler::RenderResult render(const poppler::RenderRequest& req) = 0;
    virtual poppler::ExtractResult extract(const poppler::ExtractRequest& req) = 0;
};

} // namespace core::ports::services
