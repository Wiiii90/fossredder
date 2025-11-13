#pragma once

#include "services/poppler/PopplerDTO.h"
#include <memory>
#include <string>
#include <vector>

class IDebugger;

class PopplerEngine {
public:
    static std::vector<PopplerRenderedPage> renderDocument(const std::string& pdfPath, double dpi, std::shared_ptr<IDebugger> debugger = nullptr);
    static std::vector<PopplerRenderedPage> extractDocumentMeta(const std::string& pdfPath, double dpi, std::shared_ptr<IDebugger> debugger = nullptr);
};
