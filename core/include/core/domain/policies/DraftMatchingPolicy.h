/**
 * @file core/include/core/domain/policies/DraftMatchingPolicy.h
 * @brief Declares draft matching policy helpers for import-time bank statement inference.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/application/workspace/WorkspaceState.h"

namespace core::domain::policies::matching {

std::string trim(std::string value);
bool matchesDraftText(const std::string& left, const std::string& right);
std::vector<std::string> splitLines(const std::string& text);
std::string joinNonEmptyLines(const std::vector<std::string>& values, const std::string& separator);
bool lineLooksMetadataBoundary(const std::string& line);
bool lineLooksActorStopSignal(const std::string& line);
bool lineLooksActorContinuation(const std::string& line);
std::string firstMeaningfulLine(const std::string& text);
std::string leadingPhrase(const std::string& text, std::size_t maxTokens);
std::string normalizeText(std::string text);
std::vector<std::string> tokens(const std::string& text);
std::string leadingText(const std::string& text, std::size_t maxTokens);
bool tokenIsMatchable(const std::string& token);
int tokenOverlapScore(const std::vector<std::string>& sourceTokens,
                      const std::vector<std::string>& targetTokens);
double aliasHitWeight(int hitCount);
double aliasRecencyWeight(const std::string& lastUsedAt);
std::string metadataSignalText(const std::string& metadata);
std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata);
std::vector<std::string> knownContractTypes(const core::domain::WorkspaceState& state);
std::string extractTypeText(const core::domain::WorkspaceState& state,
                            const std::string& signalText,
                            const std::vector<std::string>& lines);
std::string extractActorText(const std::vector<std::string>& lines, const std::string& typeText);

} // namespace core::domain::policies::matching

