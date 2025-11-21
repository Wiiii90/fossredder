#pragma once

#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include <memory>

namespace api::poppler {

class IPopplerAdapter {
public:
    virtual ~IPopplerAdapter() = default;

    virtual RenderResult render(const RenderRequest& req) = 0;
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

}
