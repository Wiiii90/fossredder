#pragma once

#include "OpenCvDTO.h"
#include <vector>
#include <string>
#include <memory>

class IOpenCvService {
public:
    virtual ~IOpenCvService() = default;

    virtual StatementResult processStatement(const StatementRequest& req) const = 0;
    virtual StatementBatchResult processStatementsBatch(const StatementBatchRequest& req) const = 0;
};

std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvService> adapter);
