#pragma once

#include "services/poppler/PopplerServiceTypes.h"
#include <memory>

class IDebugger;

class IPopplerAdapter {
public:
    virtual ~IPopplerAdapter() = default;

    virtual RenderResult renderDocument(const RenderRequest& req) = 0;
    virtual ExtractResult extractDocumentMeta(const ExtractRequest& req) = 0;
};

std::shared_ptr<IPopplerAdapter> createPopplerAdapter(std::shared_ptr<IDebugger> debugger = nullptr);
