/**
 * @file core/src/import/parsing/ParserConfig.h
 * @brief Declares the private parser tuning configuration shared across parser implementation files.
 */

#pragma once

namespace core::parser::helpers {

struct ParserConfig {
    int amountNearValutaBandPx = 120;
    int valutaNeighborExpandPx = 140;
    int leftDescriptiveOffsetPx = 200;
    int tokenNearMergeBandPx = 220;
    int maxPhraseTokens = 6;
    int tokenNearBandForMainRow = 100;
    int groupMergeMaxGapPx = 8;
    int headerScanLines = 12;
    int headerMarginPx = 8;
    int orphanAttachMaxGapPx = 80;
    int orphanAcceptXOffsetPx = 40;
};

extern ParserConfig parserConfig;

} // namespace core::parser::helpers
