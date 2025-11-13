#pragma once

#include "services/poppler/IPopplerAdapter.h"
#include "services/poppler/PopplerServiceTypes.h"
#include <memory>

class IPopplerService {
public:
    virtual ~IPopplerService() = default;

    virtual RenderResult render(const RenderRequest& req) = 0;
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

std::shared_ptr<IPopplerService> createPopplerService(std::shared_ptr<IPopplerAdapter> adapter);
