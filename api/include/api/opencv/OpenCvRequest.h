#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "api/opencv/Types.h"

namespace api::opencv {

struct DenoiseRequest {
    std::filesystem::path imagePath;
    enum class Method { Median, Gaussian, Bilateral } method = Method::Median;
};

struct MaskRequest {
    std::filesystem::path imagePath;
    std::filesystem::path outputDir; // required: where mask images will be written
    std::string uniqIdPrefix; // unique id for this request's outputs
    std::string filePrefix; // e.g. "<ts>_opencv_mask_page1"
    std::vector<Rect> textElements;
    enum class Mode { Whiteout, Inpaint } mode = Mode::Whiteout;
    std::string tesseractTsv;
    bool usePoppler = true;
    bool useTesseract = false;
    bool useMorphology = true;
};

struct DetectRequest {
    std::filesystem::path imagePath;
    std::filesystem::path outputDir; // required
    std::string uniqIdPrefix;
    std::string filePrefix; // optional
    enum class DetectKind { Tables, Cells, TextBlocks } kind = DetectKind::Tables;
};

struct CropRequest {
    std::filesystem::path imagePath;
    std::filesystem::path outputDir; // required: where cropped images will be written
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "<ts>_opencv_crop_table_page1"
    Rect bbox;
    enum class OutputFormat { Png, Jpg } outputFormat = OutputFormat::Png;
    int jpegQuality = 92;
};

}