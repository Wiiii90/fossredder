/**
 * @file core/tests/unit/TestExportController.cpp
 * @brief Unit tests for the core export entry point.
 */

#include "gtest/gtest.h"

#include "core/constants/CoreDefaults.h"
#include "core/controllers/ExportController.h"

TEST(ExportControllerTests, UnsupportedFormatReturnsExpectedError)
{
    core::controllers::exporting::ExportController exporter;
    core::controllers::exporting::ExportRequest request;
    request.format = static_cast<core::controllers::exporting::ExportFormat>(99);

    const auto result = exporter.exportData(request);

    EXPECT_EQ(result.status, core::controllers::exporting::ExportStatus::UnsupportedFormat);
    EXPECT_EQ(result.errorCode, core::constants::exportFlow::kErrorUnsupportedFormat);
    EXPECT_EQ(result.message, core::constants::exportFlow::kMessageUnsupportedFormat);
}

TEST(ExportControllerTests, CsvWithoutOutputPathReturnsExpectedError)
{
    core::controllers::exporting::ExportController exporter;
    core::controllers::exporting::ExportRequest request;
    request.format = core::controllers::exporting::ExportFormat::Csv;

    const auto result = exporter.exportData(request);

    EXPECT_EQ(result.status, core::controllers::exporting::ExportStatus::InvalidInput);
    EXPECT_EQ(result.errorCode, core::constants::exportFlow::errors::kOutputPathEmpty);
    EXPECT_EQ(result.message, core::constants::exportFlow::messages::kOutputPathEmpty);
}
