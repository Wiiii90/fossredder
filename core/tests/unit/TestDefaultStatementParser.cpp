/**
 * @file core/tests/unit/TestDefaultStatementParser.cpp
 * @brief End-to-end regression tests for statement parsing metadata preservation.
 */

#include "gtest/gtest.h"

#include "../../src/import/parsing/DefaultStatementParser.h"

namespace {

api::tesseract::Word makeWord(const std::string& text, int x, int y, int width = 40, int height = 10)
{
    api::tesseract::Word word;
    word.text = text;
    word.bbox.x = x;
    word.bbox.y = y;
    word.bbox.width = width;
    word.bbox.height = height;
    return word;
}

} // namespace

TEST(DefaultStatementParserTests, PreservesStandaloneMetadataLinesBelowMainRow)
{
    api::tesseract::ExtractResult ocr;
    const std::vector<uint8_t> pageCropImageBytes;
    ocr.words = {
        makeWord("Lastschrift", 10, 10, 90),
        makeWord("01.04", 200, 10, 45),
        makeWord("123,45-", 360, 10, 55),
        makeWord("SYNTHB01XXX", 205, 24, 110),
        makeWord("ZZ00SYNTHETICACCOUNT0001", 205, 38, 220),
        makeWord("Dauerauftrag", 10, 52, 95),
    };

    const auto result = DefaultStatementParser::parse({}, ocr, "", nullptr, pageCropImageBytes, "2025-04-01", 1);

    ASSERT_EQ(result.transactions.size(), 1U);
    const auto& tx = result.transactions.front();
    EXPECT_NE(tx.metadata.find("SYNTHB01XXX"), std::string::npos);
    EXPECT_NE(tx.metadata.find("ZZ00SYNTHETICACCOUNT0001"), std::string::npos);
    EXPECT_NE(tx.metadata.find("Dauerauftrag"), std::string::npos);
}

TEST(DefaultStatementParserTests, PreservesSecondMetadataLineWhenItContainsAmountAndTrailingText)
{
    api::tesseract::ExtractResult ocr;
    const std::vector<uint8_t> pageCropImageBytes;
    ocr.words = {
        makeWord("Lastschrift", 10, 10, 90),
        makeWord("01.04", 200, 10, 45),
        makeWord("123,45-", 360, 10, 55),
        makeWord("SUMME", 10, 24, 55),
        makeWord("ENTHAELT:", 80, 24, 95),
        makeWord("EUR", 10, 38, 30),
        makeWord("10,00", 205, 38, 55),
        makeWord("POSTENENTGELTE", 275, 38, 145),
    };

    const auto result = DefaultStatementParser::parse({}, ocr, "", nullptr, pageCropImageBytes, "2025-04-01", 1);

    ASSERT_EQ(result.transactions.size(), 1U);
    const auto& tx = result.transactions.front();
    EXPECT_NE(tx.metadata.find("SUMME ENTHAELT:"), std::string::npos);
    EXPECT_NE(tx.metadata.find("EUR 10,00 POSTENENTGELTE"), std::string::npos);
}

TEST(DefaultStatementParserTests, PreservesLeftStartingSecondMetadataLineWhenAmountIsRecognized)
{
    api::tesseract::ExtractResult ocr;
    const std::vector<uint8_t> pageCropImageBytes;
    ocr.words = {
        makeWord("Lastschrift", 10, 10, 90),
        makeWord("01.04", 200, 10, 45),
        makeWord("123,45-", 360, 10, 55),
        makeWord("ABSCHLAG", 10, 24, 85),
        makeWord("123,45", 205, 24, 55),
        makeWord("Dauerauftrag", 10, 38, 95),
    };

    const auto result = DefaultStatementParser::parse({}, ocr, "", nullptr, pageCropImageBytes, "2025-04-01", 1);

    ASSERT_EQ(result.transactions.size(), 1U);
    const auto& tx = result.transactions.front();
    EXPECT_NE(tx.metadata.find("ABSCHLAG 123,45"), std::string::npos);
    EXPECT_NE(tx.metadata.find("Dauerauftrag"), std::string::npos);
}

TEST(DefaultStatementParserTests, PreservesLeftStartingReferenceLineBelowMainRow)
{
    api::tesseract::ExtractResult ocr;
    const std::vector<uint8_t> pageCropImageBytes;
    ocr.words = {
        makeWord("SERVICE-INVOICE", 10, 10, 140),
        makeWord("01.04", 205, 10, 45),
        makeWord("123,45-", 360, 10, 55),
        makeWord("REFERENCE_TOKEN_ALPHA_0001", 10, 24, 230),
        makeWord("SUMMARY", 10, 38, 85),
        makeWord("ITEMS:", 105, 38, 70),
        makeWord("OVERVIEW", 10, 52, 90),
    };

    const auto result = DefaultStatementParser::parse({}, ocr, "", nullptr, pageCropImageBytes, "2025-04-01", 1);

    ASSERT_EQ(result.transactions.size(), 1U);
    const auto& tx = result.transactions.front();
    EXPECT_NE(tx.metadata.find("REFERENCE_TOKEN_ALPHA_0001"), std::string::npos);
    EXPECT_NE(tx.metadata.find("SUMMARY ITEMS:"), std::string::npos);
}

TEST(DefaultStatementParserTests, PreservesInvoiceReferenceLineBelowMainRow)
{
    api::tesseract::ExtractResult ocr;
    const std::vector<uint8_t> pageCropImageBytes;
    ocr.words = {
        makeWord("SERVICE_PROVIDER_SAMPLE_0001", 10, 10, 230),
        makeWord("01.06", 205, 10, 45),
        makeWord("123,45-", 360, 10, 55),
        makeWord("DOCUMENT_TOKEN_ALPHA_0001", 10, 24, 230),
        makeWord("End-to-End-Ref.:", 10, 38, 130),
        makeWord("NOTPROVIDED", 160, 38, 110),
    };

    const auto result = DefaultStatementParser::parse({}, ocr, "", nullptr, pageCropImageBytes, "2025-06-01", 1);

    ASSERT_EQ(result.transactions.size(), 1U);
    const auto& tx = result.transactions.front();
    EXPECT_NE(tx.metadata.find("DOCUMENT_TOKEN_ALPHA_0001"), std::string::npos);
    EXPECT_NE(tx.metadata.find("End-to-End-Ref.: NOTPROVIDED"), std::string::npos);
}
