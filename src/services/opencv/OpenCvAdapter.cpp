#include "services/opencv/IOpenCvAdapter.h"
#include "services/opencv/OpenCvDTO.h"
#include "services/opencv/OpenCvEngine.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>

class OpenCvAdapterImpl : public IOpenCvService {
public:
    explicit OpenCvAdapterImpl(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    StatementResult processStatement(const StatementRequest& req) const override {
        StatementResult res;
        res.source = req;
        try {
            for (const auto &pageReq : req.pages) {
                LayoutResult out;
                out.source = pageReq;

                // load grayscale image
                cv::Mat img = cv::imread(pageReq.imagePath, cv::IMREAD_GRAYSCALE);
                if (img.empty()) {
                    if (debugger && debugger->enabled()) debugger->writeText("open_cv/error.txt", std::string("failed to read image ") + pageReq.imagePath);
                    res.layoutBatchResult.pageOutputs.push_back(out);
                    continue;
                }

                // mask text elements
                cv::Mat masked = img.clone();
                try {
                    for (const auto &te : pageReq.textElements) {
                        int x = static_cast<int>(std::round(te.x));
                        int y = static_cast<int>(std::round(te.y));
                        int w = static_cast<int>(std::round(te.width));
                        int h = static_cast<int>(std::round(te.height));
                        cv::Rect r(x, y, w, h);
                        r &= cv::Rect(0, 0, masked.cols, masked.rows);
                        if (r.width > 0 && r.height > 0) cv::rectangle(masked, r, cv::Scalar(255), cv::FILLED);
                    }
                } catch (...) { masked = img.clone(); }

                // detect tables via engine
                auto tables = OpenCvEngine::detectTablesFromImage(masked, pageReq.imagePath, 4, debugger);
                for (const auto &t : tables) {
                    LayoutFinding lf;
                    lf.kind = LayoutFinding::Kind::Table;
                    lf.table = t;
                    lf.x = t.bbox.x; lf.y = t.bbox.y; lf.width = t.bbox.width; lf.height = t.bbox.height;
                    lf.confidence = 1.0;
                    out.findings.push_back(std::move(lf));
                }

                res.layoutBatchResult.pageOutputs.push_back(std::move(out));
            }
        } catch (...) {
            // on error, return whatever we collected
        }
        return res;
    }

    StatementBatchResult processStatementsBatch(const StatementBatchRequest& req) const override {
        StatementBatchResult res;
        res.outputs.reserve(req.statements.size());
        for (const auto &s : req.statements) {
            try { res.outputs.push_back(processStatement(s)); }
            catch (...) { res.outputs.push_back(StatementResult()); }
        }
        return res;
    }

private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<IOpenCvService> createOpenCvAdapter(std::shared_ptr<IDebugger> debugger){ return std::make_shared<OpenCvAdapterImpl>(std::move(debugger)); }
