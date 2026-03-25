/**
 * @file core/tests/unit/TestParserHelpers.cpp
 * @brief Unit tests for parser helper line preservation heuristics.
 */

#include "gtest/gtest.h"

#include "../../src/import/parsing/ParserHelpers.h"
#include "../../src/import/parsing/StatementParseHelpers.h"

namespace {

core::parser::OcrLine makeLine(const std::string& text,
                               const std::vector<std::pair<int, int>>& spans,
                               int minY = 0,
                               int maxY = 10)
{
    core::parser::OcrLine line;
    line.text = text;
    line.wordSpans = spans;
    line.minY = minY;
    line.maxY = maxY;
    if (!spans.empty()) {
        line.minX = spans.front().first;
        line.maxX = spans.back().second;
    }
    return line;
}

core::parser::helpers::detail::RawLineLite makeRawLine(const std::string& text,
                                                       const std::vector<std::pair<int, int>>& spans,
                                                       int minY,
                                                       int maxY)
{
    core::parser::helpers::detail::RawLineLite line{0, 0, minY, maxY, spans, text};
    if (!spans.empty()) {
        line.minX = spans.front().first;
        line.maxX = spans.back().second;
    }
    return line;
}

} // namespace

TEST(ParserHelpersTests, PreservesWholeIbanDetailLineWhenColumnCutWouldDropIt)
{
    core::parser::TransactionBlock block;
    core::parser::helpers::ColumnGuess cols{100, -1, -1};
    const auto ibanLine = makeLine("ZZ00SYNTHETICACCOUNT0001", {{140, 320}});

    core::parser::detail::appendDetailLine(block, ibanLine, cols, nullptr);

    ASSERT_EQ(block.detailLines.size(), 1U);
    EXPECT_EQ(block.detailLines.front().text, "ZZ00SYNTHETICACCOUNT0001");
}

TEST(ParserHelpersTests, PreservesWholeLeftStartingDetailLineWhenAmountAppearsOnRight)
{
    core::parser::TransactionBlock block;
    core::parser::helpers::ColumnGuess cols{100, -1, -1};
    const auto mixedLine = makeLine("Abschlag 123,45", {{10, 70}, {120, 165}});

    core::parser::detail::appendDetailLine(block, mixedLine, cols, nullptr);

    ASSERT_EQ(block.detailLines.size(), 1U);
    EXPECT_EQ(block.detailLines.front().text, "Abschlag 123,45");
}

TEST(ParserHelpersTests, PreservesWholeMetadataLineWhenAmountIsFollowedByDescriptiveText)
{
    core::parser::TransactionBlock block;
    core::parser::helpers::ColumnGuess cols{100, -1, -1};
    const auto mixedLine = makeLine("EUR 10,00 POSTENENTGELTE", {{10, 35}, {120, 165}, {190, 320}});

    core::parser::detail::appendDetailLine(block, mixedLine, cols, nullptr);

    ASSERT_EQ(block.detailLines.size(), 1U);
    EXPECT_EQ(block.detailLines.front().text, "EUR 10,00 POSTENENTGELTE");
}

TEST(ParserHelpersTests, DoesNotMergeMainRowWithFollowingStandaloneMetadataLine)
{
    const std::vector<core::parser::helpers::detail::RawLineLite> lines{
        makeRawLine("Lastschrift 01.04 123,45-", {{10, 95}, {200, 250}, {360, 430}}, 10, 20),
        makeRawLine("SYNTHB01XXX", {{205, 285}}, 24, 34),
    };

    const auto merged = core::parser::detail::selectiveGroupMergeLinesRaw(lines, 8, {225, 360, 460});

    ASSERT_EQ(merged.size(), 2U);
    EXPECT_EQ(merged[0].text, "Lastschrift 01.04 123,45-");
    EXPECT_EQ(merged[1].text, "SYNTHB01XXX");
}

TEST(ParserHelpersTests, DoesNotMergeMainRowWithFollowingDateOnlyMetadataLine)
{
    const std::vector<core::parser::helpers::detail::RawLineLite> lines{
        makeRawLine("Lastschrift 01.04 123,45-", {{10, 95}, {200, 250}, {360, 430}}, 10, 20),
        makeRawLine("1/6/2011", {{205, 255}}, 24, 34),
    };

    const auto merged = core::parser::detail::selectiveGroupMergeLinesRaw(lines, 8, {225, 360, 460});

    ASSERT_EQ(merged.size(), 2U);
    EXPECT_EQ(merged[0].text, "Lastschrift 01.04 123,45-");
    EXPECT_EQ(merged[1].text, "1/6/2011");
}

TEST(ParserHelpersTests, DoesNotMergeMainRowWithFollowingLeftStartingReferenceLine)
{
    const std::vector<core::parser::helpers::detail::RawLineLite> lines{
        makeRawLine("SERVICE_INVOICE_ALPHA 01.04 123,45-", {{10, 180}, {205, 250}, {360, 430}}, 10, 20),
        makeRawLine("REFERENCE_TOKEN_ALPHA_0001", {{10, 230}}, 24, 34),
    };

    const auto merged = core::parser::detail::selectiveGroupMergeLinesRaw(lines, 8, {225, 360, 460});

    ASSERT_EQ(merged.size(), 2U);
    EXPECT_EQ(merged[0].text, "SERVICE_INVOICE_ALPHA 01.04 123,45-");
    EXPECT_EQ(merged[1].text, "REFERENCE_TOKEN_ALPHA_0001");
}
