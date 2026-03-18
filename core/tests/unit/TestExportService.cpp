/**
 * @file core/tests/unit/TestExportService.cpp
 * @brief Unit tests for the core export service.
 */

#include "gtest/gtest.h"

#include "core/constants/CoreDefaults.h"
#include "core/export/ExportService.h"

TEST(ExportServiceTests, UnsupportedFormatReturnsExpectedError)
{
    core::exporting::ExportService exporter;
    core::exporting::ExportRequest request;
    request.format = static_cast<core::exporting::ExportFormat>(99);

    const auto result = exporter.exportData(request);

    EXPECT_EQ(result.status, core::exporting::ExportStatus::UnsupportedFormat);
    EXPECT_EQ(result.errorCode, core::constants::exportFlow::kErrorUnsupportedFormat);
    EXPECT_EQ(result.message, core::constants::exportFlow::kMessageUnsupportedFormat);
}

TEST(ExportServiceTests, CsvWithoutOutputPathReturnsExpectedError)
{
    core::exporting::ExportService exporter;
    core::exporting::ExportRequest request;
    request.format = core::exporting::ExportFormat::Csv;

    const auto result = exporter.exportData(request);

    EXPECT_EQ(result.status, core::exporting::ExportStatus::InvalidInput);
    EXPECT_EQ(result.errorCode, core::constants::exportFlow::errors::kOutputPathEmpty);
    EXPECT_EQ(result.message, core::constants::exportFlow::messages::kOutputPathEmpty);
}
