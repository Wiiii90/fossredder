#pragma once

#include "api/poppler/IPopplerAdapter.h"
#include "api/poppler/PopplerRequest.h"
#include "api/poppler/PopplerResponse.h"
#include <memory>

namespace api::poppler {

class IPopplerService {
public:
    virtual ~IPopplerService() = default;

    virtual RenderResult render(const RenderRequest& req) = 0;
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<class IPopplerAdapter> adapter);

}
