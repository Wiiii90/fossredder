/**
 * @file core/src/import/ImportPageRequests.h
 * @brief Declares private request-building helpers for per-page import processing.
 */

#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResponse.h"
#include "api/poppler/PopplerResponse.h"
#include "api/tesseract/TesseractRequest.h"
#include "core/constants/CoreDefaults.h"
#include "core/import/ImportRequest.h"
#include "core/utils/UniqId.h"

#include <cmath>
#include <string>
#include <vector>

namespace core::importing::detail {

inline api::opencv::MaskRequest buildMaskRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const api::poppler::ExtractResult& extractRes)
{
    api::opencv::MaskRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = utils::makeUniqId();
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
            api::opencv::Rect rect;
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

inline api::tesseract::ExtractRequest buildMaskOcrRequest(const std::vector<uint8_t>& pageBytes,
                                                          const ImportRequest& req)
{
    api::tesseract::ExtractRequest request;
    request.imageBytes = pageBytes;
    request.tessdataPath = {};
    request.psm = core::constants::importing::kDefaultTesseractPsm;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline api::opencv::DetectRequest buildDetectRequest(const std::vector<uint8_t>& maskedBytes,
                                                     size_t pageIndex,
                                                     const ImportRequest& req)
{
    api::opencv::DetectRequest request;
    request.imageBytes = maskedBytes;
    request.uniqIdPrefix = utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kOpenCvDetectPrefix) + std::to_string(pageIndex + 1);
    request.kind = api::opencv::DetectRequest::DetectKind::Tables;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline api::opencv::CropRequest buildCropRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const api::opencv::DetectResult& detectResponse)
{
    api::opencv::CropRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kOpenCvCropPrefix) + std::to_string(pageIndex + 1);
    request.bbox = detectResponse.table.bbox;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline api::tesseract::ExtractRequest buildTableOcrRequest(const std::vector<uint8_t>& croppedBytes,
                                                           size_t pageIndex,
                                                           const ImportRequest& req,
                                                           const api::opencv::DetectResult& detectResponse)
{
    api::tesseract::ExtractRequest request;
    request.kind = api::tesseract::ExtractRequest::Kind::Table;
    request.imageBytes = croppedBytes;
    request.uniqIdPrefix = utils::makeUniqId();
    request.filePrefix = std::string(core::constants::importing::kTableTesseractPrefix) + std::to_string(pageIndex + 1);
    request.tessdataPath = {};
    request.psm = core::constants::importing::kDefaultTableTesseractPsm;
    request.cancelFlag = req.cancelFlag;

    request.cells.reserve(detectResponse.table.cells.size());
    for (const auto& cell : detectResponse.table.cells) {
        api::tesseract::Cell mappedCell;
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

} // namespace core::importing::detail
