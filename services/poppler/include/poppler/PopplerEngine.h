#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include "poppler/PopplerDTO.h"

class IDebugger;

class PopplerEngine {
public:
    static std::vector<PopplerRenderedPage> renderDocument(const std::string& path, double dpi, const std::filesystem::path& outputDir = std::filesystem::path(), std::shared_ptr<IDebugger> dbg = nullptr);
    static std::vector<PopplerRenderedPage> extractDocumentMeta(const std::string& path, double dpi, const std::filesystem::path& outputDir = std::filesystem::path(), std::shared_ptr<IDebugger> dbg = nullptr);
};
