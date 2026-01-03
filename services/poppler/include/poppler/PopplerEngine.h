#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include "api/poppler/Types.h"

class IDebugger;

class PopplerEngine {
public:
    static std::vector<api::poppler::RenderedPage> renderDocument(const std::string& path,
                                                                 double dpi,
                                                                 const std::filesystem::path& outputDir = std::filesystem::path(),
                                                                 const std::string& uniqIdPrefix = "",
                                                                 const std::string& filePrefix = "",
                                                                 std::shared_ptr<IDebugger> dbg = nullptr);

    static std::vector<api::poppler::RenderedPage> extractDocumentMeta(const std::string& path,
                                                                      double dpi,
                                                                      const std::filesystem::path& outputDir = std::filesystem::path(),
                                                                      const std::string& uniqIdPrefix = "",
                                                                      const std::string& filePrefix = "",
                                                                      std::shared_ptr<IDebugger> dbg = nullptr);
};
