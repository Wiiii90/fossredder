#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "services/poppler/PopplerDTO.h"
#include "services/poppler/PopplerServiceTypes.h"
#include "services/opencv/OpenCvDTO.h"
#include "services/tesseract/TesseractDTO.h"

class Page;
class BookingGroup;

class StatementData {
public:
    StatementData(const std::string& sourceFile,
                  const std::vector<uint8_t>& sourceBytes = {},
                  const RenderResult& renderArtifact = RenderResult(),
                  const std::vector<LayoutResult>& openCvArtifacts = {},
                  const std::vector<std::vector<OcrTableDto>>& ocrArtifacts = {});


    // Source access
    const std::string& getSourceFile() const;
    const std::vector<uint8_t>& getSourceBytes() const;

    // DTOs
    const RenderResult& getRenderArtifact() const;
    const std::vector<LayoutResult>& getOpenCvArtifacts() const;
    const std::vector<std::vector<OcrTableDto>>& getOcrArtifacts() const;

private:
    std::string sourceFile;
    std::vector<uint8_t> sourceBytes;

    RenderResult renderArtifact;
    std::vector<LayoutResult> openCvArtifacts;
    std::vector<std::vector<OcrTableDto>> ocrArtifacts;
};
