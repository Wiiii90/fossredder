/**
 * @file core/src/application/import/ImportPageRequests.h
 * @brief Declares private request-building helpers for per-page import processing.
 */

#pragma once

#include "api/opencv/OpenCvRequest.h"
#include "api/opencv/OpenCvResult.h"
#include "api/poppler/PopplerResult.h"
#include "api/tesseract/TesseractRequest.h"
#include "core/constants/import.h"
#include "core/application/import/ImportRequest.h"
#include "../../utils/UniqId.h"

#include <cmath>
#include <string>
#include <vector>

namespace core::application::importing::internal {

/**
 * @brief Creates OCR recognition settings for a specific Tesseract page mode.
 * @param defaultPsm Default page segmentation mode to use.
 * @return Recognition settings configured with the requested mode.
 */
inline api::tesseract::RecognitionSettings buildRecognitionSettings(int defaultPsm)
{
    api::tesseract::RecognitionSettings settings;
    settings.psm = defaultPsm;
    return settings;
}

/**
 * @brief Builds the OpenCV mask request for one imported page.
 * @param pageBytes Raw bytes of the source page image.
 * @param pageIndex Zero-based page index.
 * @param req Current import request.
 * @param extractRes Poppler extraction result for the full document.
 * @return Mask request configured for the page.
 */
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

/**
 * @brief Builds the fallback OCR request used for masked page extraction.
 * @param pageBytes Raw bytes of the masked page image.
 * @param req Current import request.
 * @return OCR request configured for masked page analysis.
 */
inline api::tesseract::ExtractRequest buildMaskOcrRequest(const std::vector<uint8_t>& pageBytes,
                                                          const ImportRequest& req)
{
    api::tesseract::ExtractRequest request;
    request.imageBytes = pageBytes;
    request.tessdataPath = {};
    request.recognition = buildRecognitionSettings(core::constants::importing::kDefaultTesseractPsm);
    request.cancelFlag = req.cancelFlag;
    return request;
}

/**
 * @brief Builds the OpenCV table-detection request for one page.
 * @param maskedBytes Masked page image bytes.
 * @param pageIndex Zero-based page index.
 * @param req Current import request.
 * @return Table-detection request configured for the page.
 */
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

/**
 * @brief Builds the OpenCV crop request for one detected table.
 * @param pageBytes Raw bytes of the source page image.
 * @param pageIndex Zero-based page index.
 * @param req Current import request.
 * @param detectResponse Table detection result for the page.
 * @return Crop request configured for the detected table.
 */
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

/**
 * @brief Builds the OCR request used for extracting table text.
 * @param croppedBytes Cropped table image bytes.
 * @param pageIndex Zero-based page index.
 * @param req Current import request.
 * @param detectResponse Table detection result for the page.
 * @return OCR request configured for the detected table region.
 */
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
    request.recognition = buildRecognitionSettings(core::constants::importing::kDefaultTableTesseractPsm);
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

} // namespace core::application::importing::internal
