#pragma once

#include <cstddef>
#include <string_view>

namespace core::constants {

namespace runtime {
inline constexpr std::string_view kQtStyle = "Fusion";
inline constexpr std::string_view kAppDataDirectoryName = ".fossredder";
inline constexpr std::string_view kDatabaseFileName = "fossredder.db";
}

namespace ids {
inline constexpr std::size_t kStableHexLength = 32;
inline constexpr std::string_view kHexAlphabet = "0123456789abcdef";
}

namespace appState {
inline constexpr std::string_view kDefaultImportedStatementName = "Imported";
inline constexpr std::string_view kGeneratedContractPrefix = "Vertrag ";
}

namespace config {
namespace languages {
inline constexpr std::string_view kEnglish = "EN";
inline constexpr std::string_view kGerman = "DE";
inline constexpr std::string_view kFrench = "FR";
}
}

namespace analysis {
inline constexpr std::string_view kTypeTab = "tab";
inline constexpr std::string_view kTypePlot = "plot";
inline constexpr std::string_view kTypeCalc = "calc";
inline constexpr std::string_view kPlotTypeKey = "plotType";
inline constexpr std::string_view kPlotMeasureKey = "plotMeasure";
inline constexpr std::string_view kPropertiesKey = "properties";
inline constexpr std::string_view kContractTypesKey = "contractTypes";
inline constexpr int kSchemaVersion = 1;

namespace plotTypes {
inline constexpr std::string_view kPie = "pie";
inline constexpr std::string_view kHistogram = "histogram";
}

namespace plotMeasures {
inline constexpr std::string_view kCount = "count";
inline constexpr std::string_view kAverageAmount = "averageAmount";
inline constexpr std::string_view kTotalAmount = "totalAmount";
}

namespace metricKeys {
inline constexpr std::string_view kTotalAmount = "totalAmount";
inline constexpr std::string_view kRowCount = "rowCount";
inline constexpr std::string_view kMatchedTransactions = "matchedTx";
inline constexpr std::string_view kRows = "rows";
}

namespace calc {
inline constexpr std::string_view kStrategyKey = "strategy";
inline constexpr std::string_view kPercentKey = "percent";
inline constexpr std::string_view kStrategyTax = "tax";
}

namespace resultFields {
inline constexpr std::string_view kMonth = "month";
inline constexpr std::string_view kTotal = "total";
inline constexpr std::string_view kByContract = "byContract";
inline constexpr std::string_view kByProperty = "byProperty";
inline constexpr std::string_view kAmountOriginal = "amount_original";
inline constexpr std::string_view kAmountAdjusted = "amount_adjusted";
inline constexpr std::string_view kTaxPercent = "taxPercent";
inline constexpr std::string_view kTaxFactor = "taxFactor";
inline constexpr std::string_view kTransactionId = "txId";
}

namespace labels {
inline constexpr std::string_view kNoProperty = "(no-property)";
}
}

namespace filters {
inline constexpr std::string_view kDate = "date";
inline constexpr std::string_view kAmount = "amount";
inline constexpr std::string_view kContractType = "contract.type";
inline constexpr std::string_view kPropertyId = "propertyId";
inline constexpr std::string_view kUnassigned = "unassigned";

namespace operators {
inline constexpr std::string_view kGreaterEqual = ">=";
inline constexpr std::string_view kLessEqual = "<=";
inline constexpr std::string_view kNotEqual = "!=";
inline constexpr std::string_view kGreater = ">";
inline constexpr std::string_view kLess = "<";
inline constexpr std::string_view kEqual = "=";
}

namespace separators {
inline constexpr char kClause = ';';
inline constexpr char kAlternatives = '|';
inline constexpr char kList = ',';
inline constexpr char kDateParts = '.';
}
}

namespace exportFlow {
inline constexpr std::string_view kErrorCsvControllerMissing = "EXPORT_CSV_CONTROLLER_MISSING";
inline constexpr std::string_view kErrorXlsxControllerMissing = "EXPORT_XLSX_CONTROLLER_MISSING";
inline constexpr std::string_view kErrorUnsupportedFormat = "EXPORT_UNSUPPORTED_FORMAT";
inline constexpr std::string_view kMessageCsvControllerMissing = "CSV controller is not available";
inline constexpr std::string_view kMessageXlsxControllerMissing = "XLSX controller is not available";
inline constexpr std::string_view kMessageUnsupportedFormat = "Unsupported export format";

namespace errors {
inline constexpr std::string_view kOutputPathEmpty = "EXPORT_OUTPUT_PATH_EMPTY";
inline constexpr std::string_view kStateMissing = "EXPORT_STATE_MISSING";
inline constexpr std::string_view kFileOpenFailed = "EXPORT_FILE_OPEN_FAILED";
inline constexpr std::string_view kFileWriteFailed = "EXPORT_FILE_WRITE_FAILED";
inline constexpr std::string_view kXlsxGenerationFailed = "EXPORT_XLSX_GENERATION_FAILED";
inline constexpr std::string_view kInternalError = "EXPORT_INTERNAL_ERROR";
}

namespace messages {
inline constexpr std::string_view kOutputPathEmpty = "Output path is empty";
inline constexpr std::string_view kStateMissing = "State snapshot is missing";
inline constexpr std::string_view kFileOpenFailed = "Failed to open export output file";
inline constexpr std::string_view kFileWriteFailed = "Failed while writing export output file";
inline constexpr std::string_view kXlsxGenerationFailed = "XLSX generation failed";
inline constexpr std::string_view kInternalError = "Unexpected error during export";
}

namespace labels {
inline constexpr std::string_view kPropertyHeader = "Gebäude";
inline constexpr std::string_view kTotal = "Summe";
inline constexpr std::string_view kUnassigned = "(Unassigned)";
}
}

namespace jobs {
inline constexpr std::size_t kFallbackWorkerCount = 4;
inline constexpr std::size_t kQueueCapacity = 128;
inline constexpr std::size_t kJobHistoryLimit = 64;
inline constexpr std::size_t kOcrWorkerDivisor = 2;

namespace messages {
inline constexpr std::string_view kQueued = "Queued";
inline constexpr std::string_view kRunning = "Running";
inline constexpr std::string_view kFinished = "Finished";
inline constexpr std::string_view kFailed = "Failed";
inline constexpr std::string_view kCanceled = "Canceled";
inline constexpr std::string_view kImportServiceUnavailable = "core::domain::Statement import service not available";
inline constexpr std::string_view kUnknownError = "Unknown error";
}
}

namespace importing {
inline constexpr double kRenderDpi = 300.0;
inline constexpr double kProgressPreparing = 0.02;
inline constexpr double kProgressRendering = 0.05;
inline constexpr double kProgressRendered = 0.15;
inline constexpr double kProgressExtracting = 0.16;
inline constexpr double kProgressExtracted = 0.22;
inline constexpr double kProgressPageWorkBase = 0.22;
inline constexpr double kProgressPageWorkSpan = 0.60;
inline constexpr double kProgressFinalizeBase = 0.82;
inline constexpr double kProgressFinalizeSpan = 0.18;
inline constexpr std::size_t kLocalSchedulerWorkers = 4;
inline constexpr std::size_t kLocalSchedulerQueueCapacity = jobs::kQueueCapacity;
inline constexpr std::size_t kLocalOcrSlots = 2;
inline constexpr std::size_t kUnitsPerPage = 4;
inline constexpr int kDefaultTesseractPsm = 3;
inline constexpr int kDefaultTableTesseractPsm = 6;
inline constexpr std::string_view kRunNameImport = "import";
inline constexpr std::string_view kPopplerRenderPrefix = "poppler_render";
inline constexpr std::string_view kPopplerExtractPrefix = "poppler_extract";
inline constexpr std::string_view kOpenCvMaskPrefix = "opencv_mask_page";
inline constexpr std::string_view kOpenCvDetectPrefix = "opencv_detect_tables_page";
inline constexpr std::string_view kOpenCvCropPrefix = "opencv_crop_table_page";
inline constexpr std::string_view kProgressCanceled = "Canceled";
inline constexpr std::string_view kProgressPreparingMessage = "Preparing import";
inline constexpr std::string_view kProgressRenderingMessage = "Rendering pages";
inline constexpr std::string_view kProgressRenderedMessage = "Rendered pages";
inline constexpr std::string_view kProgressExtractingMessage = "Extracting text";
inline constexpr std::string_view kProgressExtractedMessage = "Extracted text";
inline constexpr std::string_view kProgressDoneMessage = "Done";
inline constexpr std::string_view kErrorSourceMissing = "Source statement file does not exist";
inline constexpr std::string_view kErrorRunRootMissing = "Import run root is not configured";
inline constexpr std::string_view kErrorExtractionFailed = "core::domain::Statement extraction failed";
inline constexpr std::string_view kMetricsArtifactName = "metrics.json";
inline constexpr std::string_view kTableTesseractPrefix = "tesseract_extract_table_page";

namespace pageSteps {
inline constexpr std::string_view kNoImage = "No image";
inline constexpr std::string_view kMask = "Mask";
inline constexpr std::string_view kDetect = "Detect";
inline constexpr std::string_view kNoTable = "No table";
inline constexpr std::string_view kCrop = "Crop";
inline constexpr std::string_view kNoCrop = "No crop";
inline constexpr std::string_view kOcr = "OCR";
inline constexpr std::string_view kOcrFailed = "OCR failed";
inline constexpr std::string_view kDone = "Done";
inline constexpr std::string_view kFinalize = "Finalize";
}

namespace metrics {
inline constexpr std::string_view kJobId = "jobId";
inline constexpr std::string_view kSourcePath = "sourcePath";
inline constexpr std::string_view kPagesTotal = "pagesTotal";
inline constexpr std::string_view kPagesWithTable = "pagesWithTable";
inline constexpr std::string_view kRenderSeconds = "renderSec";
inline constexpr std::string_view kExtractSeconds = "extractSec";
inline constexpr std::string_view kFinalizeSeconds = "finalizeSec";
inline constexpr std::string_view kPageWorkSecondsMax = "pageWorkSecMax";
inline constexpr std::string_view kPageWorkSecondsSum = "pageWorkSecSum";
inline constexpr std::string_view kOcrSecondsSum = "ocrSecSum";
inline constexpr std::string_view kOcrWordsTotal = "ocrWordsTotal";
inline constexpr std::string_view kTotalSeconds = "totalSec";
inline constexpr std::string_view kPages = "pages";
inline constexpr std::string_view kIndex = "index";
inline constexpr std::string_view kHasTable = "hasTable";
inline constexpr std::string_view kOcrWords = "ocrWords";
}
}

namespace parser {
inline constexpr int kAttachOrphanMaxDistance = 80;
}

namespace comparison {
inline constexpr double kTransactionAmountTolerance = 0.01;
}

}
