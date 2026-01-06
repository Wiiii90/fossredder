# Service APIs — FOSSredder

Date: 2026-01-06
Author: Wilhelm Altemeier
Status: Draft

## Poppler API
Headers: `api/include/api/poppler/*`

Requests
- `RenderRequest`
  - `std::filesystem::path pdfPath`
  - `double dpi` (default 300.0)
  - `std::filesystem::path outputDir` (required)
  - `std::string uniqIdPrefix`
  - `std::string filePrefix`

- `ExtractRequest`
  - `std::filesystem::path pdfPath`
  - `double dpi` (default 300.0)
  - `int fromPage` (0-based, default 0)
  - `int toPage` (inclusive, -1 = to end)
  - `std::filesystem::path outputDir` (required)
  - `std::string uniqIdPrefix`
  - `std::string filePrefix`

Responses / Types
- `RenderResult`
  - `std::vector<std::filesystem::path> images`

- `ExtractResult`
  - `std::vector<RenderedPage> pages`

- `RenderedPage`
  - `std::string imagePath`
  - `std::string metadataJson`
  - `double pageWidthPts`
  - `double pageHeightPts`
  - `double dpiX`
  - `double dpiY`
  - `std::vector<TextElement> textElements`

- `TextElement`
  - `std::string text`
  - `double x`
  - `double y`
  - `double width`
  - `double height`

Service surface
- `RenderResult render(const RenderRequest& req);`
- `ExtractResult extract(const ExtractRequest& req);`

## OpenCV API
Headers: `api/include/api/opencv/*`

Requests / Types
- `DenoiseRequest`
  - `std::filesystem::path imagePath`
  - `enum Method { Median, Gaussian, Bilateral } method`

- `MaskRequest`
  - `std::filesystem::path imagePath`
  - `std::filesystem::path outputDir` (required)
  - `std::string uniqIdPrefix`
  - `std::string filePrefix`
  - `std::vector<Rect> textElements`
  - `enum Mode { Whiteout, Inpaint } mode`
  - `std::string tesseractTsv`
  - `bool usePoppler`
  - `bool useTesseract`
  - `bool useMorphology`

- `DetectRequest`
  - `std::filesystem::path imagePath`
  - `std::filesystem::path outputDir` (required)
  - `std::string uniqIdPrefix`
  - `std::string filePrefix`
  - `enum DetectKind { Tables, Cells, TextBlocks } kind`

- `CropRequest`
  - `std::filesystem::path imagePath`
  - `std::filesystem::path outputDir` (required)
  - `std::string uniqIdPrefix`
  - `std::string filePrefix`
  - `Rect bbox`
  - `enum OutputFormat { Png, Jpg } outputFormat`
  - `int jpegQuality`

Responses
- `DenoiseResult { std::filesystem::path denoisedImagePath; }`
- `MaskResult { std::filesystem::path maskedImagePath; std::filesystem::path maskImagePath; }`
- `DetectResult { Table table; bool detected; std::vector<Rect> textBlocks; }`
- `CropResult { std::vector<std::filesystem::path> croppedImagePaths; }`

Service surface
- `DenoiseResult denoise(const DenoiseRequest& req);`
- `MaskResult mask(const MaskRequest& req);`
- `DetectResult detect(const DetectRequest& req);`
- `CropResult crop(const CropRequest& req);`

## Tesseract API
Headers: `api/include/api/tesseract/*`

Types
- `Rect { int x; int y; int width; int height; }`
- `Text { std::string text; int meanConfidence; }`
- `Word { Rect bbox; std::string text; int confidence; }`
- `Cell { Rect bbox; int row; int col; std::string text; int confidence; }`
- `Table { Rect bbox; int rows; int cols; std::vector<Cell> cells; std::vector<Word> words; }`

Request / Response
- `ExtractRequest` (typical fields: `imagePath`, `outputDir`, `uniqIdPrefix`, `filePrefix`)
- `ExtractResult`
  - `std::string text`
  - `std::string tsv`
  - `std::vector<Word> words`
  - `std::vector<Table> tables`

Service surface
- `ExtractResult extract(const ExtractRequest& req);`