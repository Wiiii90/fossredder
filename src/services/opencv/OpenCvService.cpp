#include "pch.h"
#include "services/opencv/IOpenCvService.h"
#include "services/opencv/OpenCvDTO.h"
#include "services/opencv/IOpenCvAdapter.h"
#include <memory>

class OpenCvServiceImpl : public IOpenCvService {
public:
    OpenCvServiceImpl(std::shared_ptr<IOpenCvService> adapter) : adapter_(std::move(adapter)) {}

    StatementResult processStatement(const StatementRequest& req) const override {
        StatementResult res;
        res.source = req;
        if (!adapter_) return res;
        return adapter_->processStatement(req);
    }

    StatementBatchResult processStatementsBatch(const StatementBatchRequest& req) const override {
        if (!adapter_) {
            StatementBatchResult empty;
            empty.outputs.reserve(req.statements.size());
            return empty;
        }
        return adapter_->processStatementsBatch(req);
    }

private:
    std::shared_ptr<IOpenCvService> adapter_;
};

std::shared_ptr<IOpenCvService> createOpenCvService(std::shared_ptr<IOpenCvService> adapter) {
    return std::make_shared<OpenCvServiceImpl>(std::move(adapter));
}
