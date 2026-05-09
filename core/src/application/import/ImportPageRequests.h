/**
 * @file core/src/application/import/ImportPageRequests.h
 * @brief Declares private request-building helpers for per-page import processing.
 */

#pragma once

#include "core/ports/image-processing/OpenCvRequest.h"
#include "core/ports/image-processing/OpenCvResult.h"
#include "core/ports/pdf-rendering/PopplerResult.h"
#include "core/ports/text-recognition/TesseractRequest.h"
#include "core/constants/import.h"
#include "core/application/import/ImportRequest.h"
#include "../../utils/UniqId.h"

#include <cmath>
#include <string>
#include <vector>

namespace core::application::importing::internal {

inline core::ports::text_recognition::tesseract::RecognitionSettings buildRecognitionSettings(int defaultPsm)
{
    core::ports::text_recognition::tesseract::RecognitionSettings settings;
    settings.psm = defaultPsm;
    return settings;
}

inline core::ports::image_processing::opencv::MaskRequest buildMaskRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const core::ports::pdf_rendering::poppler::ExtractResult& extractRes)
{
    core::ports::image_processing::opencv::MaskRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kOpenCvMaskPrefix) + std::to_string(pageIndex + 1);
    request.usePoppler = true;
    request.useMorphology = true;
    request.useTesseract = false;
    request.cancelFlag = req.cancelFlag;

    if (pageIndex < extractRes.pages.size()) {
        const auto& pageExtract = extractRes.pages[pageIndex];
        const double scaleX = pageExtract.dpiX / 72.0;
        const double scaleY = pageExtract.dpiY / 72.0;
        for (const auto& textElement : pageExtract.textElements) {
            core::ports::image_processing::opencv::Rect rect;
            rect.x = static_cast<int>(std::round(textElement.x * scaleX));
            rect.y = static_cast<int>(std::round(textElement.y * scaleY));
            rect.width = static_cast<int>(std::round(textElement.width * scaleX));
            rect.height = static_cast<int>(std::round(textElement.height * scaleY));
            if (rect.width <= 1 || rect.height <= 1) continue;
            request.textElements.push_back(rect);
        }
    }

    if (request.textElements.empty()) request.useTesseract = true;
    return request;
}

inline core::ports::text_recognition::tesseract::ExtractRequest buildMaskOcrRequest(const std::vector<uint8_t>& pageBytes,
                                                          const ImportRequest& req)
{
    core::ports::text_recognition::tesseract::ExtractRequest request;
    request.imageBytes = pageBytes;
    request.tessdataPath = {};
    request.recognition = buildRecognitionSettings(core::constants::importing::kDefaultTesseractPsm);
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline core::ports::image_processing::opencv::DetectRequest buildDetectRequest(const std::vector<uint8_t>& maskedBytes,
                                                     size_t pageIndex,
                                                     const ImportRequest& req)
{
    core::ports::image_processing::opencv::DetectRequest request;
    request.imageBytes = maskedBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kOpenCvDetectPrefix) + std::to_string(pageIndex + 1);
    request.kind = core::ports::image_processing::opencv::DetectRequest::DetectKind::Tables;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline core::ports::image_processing::opencv::CropRequest buildCropRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const core::ports::image_processing::opencv::DetectResult& detectResponse)
{
    core::ports::image_processing::opencv::CropRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kOpenCvCropPrefix) + std::to_string(pageIndex + 1);
    request.bbox = detectResponse.table.bbox;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline core::ports::text_recognition::tesseract::ExtractRequest buildTableOcrRequest(const std::vector<uint8_t>& croppedBytes,
                                                           size_t pageIndex,
                                                           const ImportRequest& req,
                                                           const core::ports::image_processing::opencv::DetectResult& detectResponse)
{
    core::ports::text_recognition::tesseract::ExtractRequest request;
    request.kind = core::ports::text_recognition::tesseract::ExtractRequest::Kind::Table;
    request.imageBytes = croppedBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kTableTesseractPrefix) + std::to_string(pageIndex + 1);
    request.tessdataPath = {};
    request.recognition = buildRecognitionSettings(core::constants::importing::kDefaultTableTesseractPsm);
    request.cancelFlag = req.cancelFlag;

    request.cells.reserve(detectResponse.table.cells.size());
    for (const auto& cell : detectResponse.table.cells) {
        core::ports::text_recognition::tesseract::Cell mappedCell;
        mappedCell.row = cell.row;
        mappedCell.col = cell.col;
        mappedCell.bbox.x = cell.bbox.x - detectResponse.table.bbox.x;
        mappedCell.bbox.y = cell.bbox.y - detectResponse.table.bbox.y;
        mappedCell.bbox.width = cell.bbox.width;
        mappedCell.bbox.height = cell.bbox.height;
        request.cells.push_back(mappedCell);
    }

    return request;
}

}
