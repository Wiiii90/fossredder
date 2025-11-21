#pragma once

#include <string>
#include <vector>
#include <map>
#include <opencv2/core.hpp>

struct LayoutTextElement { std::string text; double x = 0.0; double y = 0.0; double width = 0.0; double height = 0.0; std::string font; double size = 0.0; };

struct Cell { cv::Rect bbox; int row = -1; int col = -1; std::string text; int confidence = 0; };
struct Table { cv::Rect bbox; int rows = 0; int cols = 0; std::vector<Cell> cells; void clear() { cells.clear(); rows = cols = 0; bbox = cv::Rect(); } void addCell(const Cell& c) { cells.push_back(c); } };

struct LayoutFinding { enum class Kind { Table, Mask }; Kind kind = Kind::Table; int x = 0; int y = 0; int width = 0; int height = 0; double confidence = 0.0; Table table; };

struct LayoutRequest { std::string imagePath; int widthPx = 0; int heightPx = 0; double dpiX = 72.0; double dpiY = 72.0; double pageWidthPts = 0.0; double pageHeightPts = 0.0; std::vector<LayoutTextElement> textElements; };
struct LayoutResult { LayoutRequest source; std::vector<LayoutFinding> findings; std::vector<std::string> generatedFiles; std::map<std::string, std::string> metadata; std::map<std::string, std::vector<uint8_t>> generatedFileBlobs; };

struct LayoutBatchRequest { std::vector<LayoutRequest> pages; std::string requestId; int maxTablesPerPage = 4; };
struct LayoutBatchResult { std::vector<LayoutResult> pageOutputs; std::vector<std::vector<Table>> tablesPerPage; std::map<std::string, std::string> metadata; };

// Statement-level DTOs: a Statement consists of multiple pages (LayoutRequest per page).
struct StatementRequest {
    std::string sourceFile;                     // original PDF path or identifier (optional)
    std::vector<LayoutRequest> pages;           // rendered pages belonging to the statement
    std::string requestId;                      // optional request id
};

struct StatementResult {
    StatementRequest source;
    LayoutBatchResult layoutBatchResult;        // results per page
    std::map<std::string, std::string> metadata;
};

struct StatementBatchRequest {
    std::vector<StatementRequest> statements;
    std::string batchId;
};

struct StatementBatchResult {
    std::vector<StatementResult> outputs;
    std::map<std::string, std::string> metadata;
};
