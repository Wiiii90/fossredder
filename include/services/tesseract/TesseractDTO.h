#pragma once

#include <string>
#include <vector>
#include <map>

struct RectDto { int x = 0; int y = 0; int width = 0; int height = 0; };

struct OcrWordDto { RectDto bbox; std::string text; int confidence = 0; };
struct OcrWordsDto { std::vector<OcrWordDto> words; int meanConfidence = 0; };
struct OcrTextDto { std::string text; int meanConfidence = 0; };

struct OcrCellDto { RectDto bbox; int row = 0; int col = 0; std::string text; int confidence = 0; };
struct OcrTableDto { RectDto bbox; int rows = 0; int cols = 0; std::vector<OcrCellDto> cells; std::vector<OcrWordDto> words; };

// OcrRequest now includes optional table extraction parameters so service API can take a single request
struct OcrRequest {
    std::vector<uint8_t> data; int width = 0; int height = 0; int channels = 0; int step = 0;
    // optional table extraction params
    RectDto tableBbox; // if empty (0,0,0,0) adapter may attempt full-page extraction
    std::vector<RectDto> templateCells; // relative to tableBbox
    std::string tessdataPath;
    int psm = 3; // tesseract page seg mode
    std::string debugPrefix;
}; using OcrInputDto = OcrRequest; // backward-compatible alias

struct OcrResult { OcrTextDto textResult; OcrWordsDto wordsResult; OcrTableDto tableResult; };

struct OcrBatchRequest { std::vector<std::vector<OcrRequest>> inputsPerPage; std::string tessdataPath; std::string requestId; };
struct OcrBatchResult { std::vector<std::vector<OcrTableDto>> outputsPerPage; std::map<std::string, std::string> metadata; };
