#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <atomic>
#include "api/opencv/Types.h"

namespace api::opencv {

struct DenoiseRequest {
    std::filesystem::path imagePath;
    enum class Method { Median, Gaussian, Bilateral } method = Method::Median;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct MaskRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir; // optional: where mask images will be written
    std::string uniqIdPrefix; // unique id for this request's outputs
    std::string filePrefix; // e.g. "<ts>_opencv_mask_page1"
    std::vector<Rect> textElements;
    enum class Mode { Whiteout, Inpaint } mode = Mode::Whiteout;
    std::string tesseractTsv;
    bool usePoppler = true;
    bool useTesseract = false;
    bool useMorphology = true;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct DetectRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir; // optional
    std::string uniqIdPrefix;
    std::string filePrefix; // optional
    enum class DetectKind { Tables, Cells, TextBlocks } kind = DetectKind::Tables;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct CropRequest {
    std::filesystem::path imagePath;
    std::vector<uint8_t> imageBytes;
    std::filesystem::path outputDir; // optional: where cropped images will be written
    std::string uniqIdPrefix;
    std::string filePrefix; // e.g. "<ts>_opencv_crop_table_page1"
    Rect bbox;
    enum class OutputFormat { Png, Jpg } outputFormat = OutputFormat::Png;
    int jpegQuality = 92;

    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

}