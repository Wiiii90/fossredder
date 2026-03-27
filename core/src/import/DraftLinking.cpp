/**
 * @file core/src/import/DraftLinking.cpp
 * @brief Implements import-draft matching helpers shared by the import workflow and draft UI binding.
 */

#include "core/pch.h"
#include "core/import/DraftLinking.h"

namespace core::importing {

namespace {

constexpr double kSyntheticConfidence = 0.35;

char lowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::string current;
    for (char c : text) {
        if (c == '\r') continue;
        if (c == '\n') {
            const auto line = trim(current);
            if (!line.empty()) lines.push_back(line);
            current.clear();
            continue;
        }
        current.push_back(c);
    }
    const auto tail = trim(current);
    if (!tail.empty()) lines.push_back(tail);
    return lines;
}

std::string joinNonEmptyLines(const std::vector<std::string>& values, const std::string& separator)
{
    std::ostringstream out;
    bool first = true;
    for (const auto& value : values) {
        const auto trimmedValue = trim(value);
        if (trimmedValue.empty()) continue;
        if (!first) out << separator;
        first = false;
        out << trimmedValue;
    }
    return out.str();
}

bool lineLooksMetadataBoundary(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return true;
    if (value.find(':') != std::string::npos) return true;

    static const std::regex amountPattern("(eur|usd|chf|gbp)\\s*[-+]?\\d|\\b\\d+(?:[\\.,]\\d{2})\\b", std::regex::icase);
    return std::regex_search(value, amountPattern);
}

bool lineLooksActorStopSignal(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return true;
    if (lineLooksMetadataBoundary(value)) return true;

    static const std::regex ibanPattern("^[A-Z]{2}\\d{2}[A-Z0-9]{10,32}$", std::regex::icase);
    static const std::regex bicPattern("^[A-Z]{6}[A-Z0-9]{2}(?:[A-Z0-9]{3})?$", std::regex::icase);
    static const std::regex dateOnlyPattern("^\\d{1,2}[\\./-]\\d{1,2}[\\./-]\\d{2,4}$", std::regex::icase);
    static const std::regex bankOrReferencePattern("(end-to-end|kundenreferenz|verwendungszweck|vertragskont|iban|bic|dauerauftrag|mandat|referenz|zweck)", std::regex::icase);
    return std::regex_match(value, ibanPattern)
        || std::regex_match(value, bicPattern)
        || std::regex_match(value, dateOnlyPattern)
        || std::regex_search(value, bankOrReferencePattern);
}

bool lineLooksActorContinuation(const std::string& line)
{
    const auto value = trim(line);
    if (value.empty()) return false;
    if (lineLooksActorStopSignal(value)) return false;

    int digitCount = 0;
    int alphaCount = 0;
    for (const unsigned char ch : value) {
        if (std::isdigit(ch)) ++digitCount;
        if (std::isalpha(ch)) ++alphaCount;
    }

    if (alphaCount == 0) return false;
    if (digitCount == 0) return true;
    if (digitCount <= 2 && alphaCount >= digitCount * 2) return true;

    static const std::regex compactReferencePattern("^[A-Z]{1,6}(?:[ -]?[A-Z0-9]{2,8}){1,5}$", std::regex::icase);
    return value.size() <= 36 && std::regex_match(value, compactReferencePattern);
}

std::string metadataSignalText(const std::string& metadata);

std::vector<std::string> knownContractTypes(const core::domain::AppState& state)
{
    std::vector<std::string> types;
    std::vector<std::string> normalizedSeen;
    types.reserve(state.contracts.size());
    normalizedSeen.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        const auto type = trim(contract->type);
        const auto normalizedType = normalizeDraftText(type);
        if (normalizedType.empty()) continue;
        if (std::find(normalizedSeen.begin(), normalizedSeen.end(), normalizedType) != normalizedSeen.end()) continue;
        normalizedSeen.push_back(normalizedType);
        types.push_back(type);
    }
    return types;
}

std::string extractTypeText(const core::domain::AppState& state,
                            const std::string& signalText,
                            const std::vector<std::string>& lines)
{
    const auto contractTypes = knownContractTypes(state);
    if (contractTypes.empty()) return {};

    for (const auto& line : lines) {
        const auto value = trim(line);
        const auto separatorPos = value.find('-');
        if (value.empty() || separatorPos == std::string::npos) continue;

        const auto left = trim(value.substr(0, separatorPos));
        if (left.empty()) continue;

        const auto normalizedLeft = normalizeDraftText(left);
        for (const auto& type : contractTypes) {
            if (normalizeDraftText(type) == normalizedLeft) return type;
        }
    }

    const auto fullText = normalizeDraftText(signalText);
    const auto headText = normalizeDraftText(joinNonEmptyLines(lines, " "));

    std::string bestType;
    int bestScore = std::numeric_limits<int>::min();
    for (const auto& type : contractTypes) {
        const auto normalizedType = normalizeDraftText(type);
        if (normalizedType.empty()) continue;

        int score = 0;
        const auto fullPos = fullText.find(normalizedType);
        if (fullPos != std::string::npos) {
            score += 500;
            score += std::max(0, 120 - static_cast<int>(fullPos));
        }

        const auto headPos = headText.find(normalizedType);
        if (headPos != std::string::npos) {
            score += 220;
            score += std::max(0, 80 - static_cast<int>(headPos));
        }

        if (score <= 0) continue;
        score += static_cast<int>(normalizedType.size());
        if (score > bestScore) {
            bestScore = score;
            bestType = type;
        }
    }

    return bestType;
}

std::string extractActorText(const std::vector<std::string>& lines, const std::string& typeText)
{
    std::vector<std::string> actorLines;
    actorLines.reserve(lines.size());

    const auto normalizedType = normalizeDraftText(typeText);
    for (std::size_t index = 0; index < lines.size(); ++index) {
        const auto& line = lines[index];
        const auto value = trim(line);
        if (value.empty()) continue;

        if (actorLines.empty()) {
            actorLines.push_back(value);
            continue;
        }

        if (lineLooksActorStopSignal(value)) break;
        if (!lineLooksActorContinuation(value)) break;
        actorLines.push_back(value);
        if (index >= 1) break;
    }

    if (actorLines.empty()) return {};

    auto actorText = joinNonEmptyLines(actorLines, " ");
    if (!normalizedType.empty() && normalizeDraftText(actorText) == normalizedType && actorLines.size() > 1) {
        actorText = joinNonEmptyLines({actorLines.front(), actorLines[1]}, " ");
    }
    return trim(actorText);
}

std::string firstMeaningfulLine(const std::string& text)
{
    for (const auto& line : splitLines(text)) {
        if (!line.empty()) return line;
    }
    return {};
}

std::string leadingPhrase(const std::string& text, std::size_t maxTokens)
{
    std::istringstream in(text);
    std::ostringstream out;
    std::string token;
    std::size_t count = 0;
    while (in >> token) {
        if (count++ > 0) out << ' ';
        out << token;
        if (count >= maxTokens) break;
    }
    return trim(out.str());
}

std::string normalizeText(std::string text)
{
    for (auto& ch : text) {
        const unsigned char byte = static_cast<unsigned char>(ch);
        if (byte < 128 && std::isalpha(byte)) {
            ch = static_cast<char>(std::tolower(byte));
        } else if (byte < 128 && std::isalnum(byte)) {
            ch = static_cast<char>(byte);
        } else {
            ch = ' ';
        }
    }
    return trim(text);
}

std::vector<std::string> tokens(const std::string& text)
{
    std::vector<std::string> out;
    std::istringstream in(normalizeText(text));
    for (std::string token; in >> token;) out.push_back(token);
    return out;
}

std::string leadingText(const std::string& text, std::size_t maxTokens)
{
    const auto parts = tokens(text);
    std::ostringstream out;
    for (std::size_t i = 0; i < parts.size() && i < maxTokens; ++i) {
        if (i > 0) out << ' ';
        out << parts[i];
    }
    return trim(out.str());
}

bool tokenIsMatchable(const std::string& token)
{
    if (token.size() >= 3) return true;
    return std::all_of(token.begin(), token.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

int tokenOverlapScore(const std::vector<std::string>& sourceTokens, const std::vector<std::string>& targetTokens)
{
    if (sourceTokens.empty() || targetTokens.empty()) return 0;
    int matches = 0;
    for (const auto& token : targetTokens) {
        if (!tokenIsMatchable(token)) continue;
        if (std::find(sourceTokens.begin(), sourceTokens.end(), token) != sourceTokens.end()) ++matches;
    }
    return matches;
}

double aliasHitWeight(int hitCount)
{
    return std::min(0.7, std::log1p(std::max(1, hitCount)) / 5.0);
}

double aliasRecencyWeight(const std::string& lastUsedAt)
{
    (void)lastUsedAt;
    return 0.0;
}

template <typename TEntity>
std::vector<core::domain::AliasUsage> aliasUsages(const TEntity& entity)
{
    if (!entity.aliasUsage.empty()) return entity.aliasUsage;

    std::vector<core::domain::AliasUsage> out;
    out.reserve(entity.aliases.size());
    for (const auto& alias : entity.aliases) {
        if (alias.empty()) continue;
        out.push_back(core::domain::AliasUsage{alias, 1, {}, {}, {}});
    }
    return out;
}

std::string metadataSignalText(const std::string& metadata)
{
    std::ostringstream out;
    bool first = true;
    const auto lines = splitLines(metadata);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        const auto colon = line.find(':');
        if (colon != std::string::npos) {
            const auto key = trim(line.substr(0, colon));
            const auto value = trim(line.substr(colon + 1));
            if (!key.empty()) {
                if (!first) out << ' ';
                first = false;
                out << key;
            }
            if (!value.empty()) {
                if (!first) out << ' ';
                first = false;
                out << value;
            } else if (i + 1 < lines.size() && !trim(lines[i + 1]).empty()) {
                if (!first) out << ' ';
                first = false;
                out << trim(lines[i + 1]);
                ++i;
            }
            continue;
        }

        if (!first) out << ' ';
        first = false;
        out << line;
    }
    return trim(out.str());
}

std::string firstMeaningfulToken(const std::string& text)
{
    const auto parts = tokens(text);
    for (const auto& part : parts) {
        if (!part.empty()) return part;
    }
    return {};
}

std::string joinStrings(const std::vector<std::string>& values, const std::string& separator)
{
    std::ostringstream out;
    bool first = true;
    for (const auto& value : values) {
        if (value.empty()) continue;
        if (!first) out << separator;
        first = false;
        out << value;
    }
    return out.str();
}

bool containsId(const std::vector<std::string>& ids, const std::string& id)
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

bool appendUnique(std::vector<std::string>& values, const std::string& value)
{
    if (value.empty() || containsId(values, value)) return false;
    values.push_back(value);
    return true;
}

const DraftSuggestionCandidate* topSuggestion(const DraftSuggestionBucket& bucket)
{
    return bucket.candidates.empty() ? nullptr : &bucket.candidates.front();
}

int confidencePercent(const DraftSuggestionCandidate* suggestion)
{
    return suggestion ? static_cast<int>(std::round(std::clamp(suggestion->confidence, 0.0, 1.0) * 100.0)) : 0;
}

std::string proofSourceFor(const std::string& path)
{
    if (path.empty()) return {};
    if (path.rfind("proof/", 0) == 0) return std::string("image://importProof/") + path;
    if (path.rfind("file://", 0) == 0) return path;

    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    if (normalized.rfind("//", 0) == 0) return std::string("file:") + normalized;
    if (normalized.size() >= 2 && normalized[1] == ':') return std::string("file:///") + normalized;
    return std::string("file:///") + normalized;
}

std::string actorDisplay(const core::domain::Actor& actor)
{
    return actor.type.empty() ? actor.name : actor.name + " — " + actor.type;
}

std::string propertyDisplay(const core::domain::Property& property)
{
    return property.address.empty() ? property.name : property.name + " — " + property.address;
}

DraftChoiceRow actorRow(const std::shared_ptr<core::domain::Actor>& actor)
{
    DraftChoiceRow row;
    if (!actor) return row;
    row.id = actor->id;
    row.name = actor->name;
    row.display = actorDisplay(*actor);
    row.type = actor->type;
    row.aliases = actor->aliases;
    return row;
}

DraftChoiceRow propertyRow(const std::shared_ptr<core::domain::Property>& property)
{
    DraftChoiceRow row;
    if (!property) return row;
    row.id = property->id;
    row.name = property->name;
    row.display = propertyDisplay(*property);
    row.aliases = property->aliases;
    return row;
}

DraftChoiceRow contractRow(const std::shared_ptr<core::domain::Contract>& contract)
{
    DraftChoiceRow row;
    if (!contract) return row;
    row.id = contract->id;
    row.name = contract->name;
    row.display = contract->name;
    row.type = contract->type;
    row.aliases = contract->aliases;
    row.actorIds = contract->actorIds;
    row.propertyIds = contract->propertyIds;
    return row;
}

std::vector<DraftChoiceRow> actorRows(const core::domain::AppState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.actors.size());
    for (const auto& actor : state.actors) {
        auto row = actorRow(actor);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<DraftChoiceRow> propertyRows(const core::domain::AppState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.properties.size());
    for (const auto& property : state.properties) {
        auto row = propertyRow(property);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<DraftChoiceRow> contractRows(const core::domain::AppState& state)
{
    std::vector<DraftChoiceRow> rows;
    rows.reserve(state.contracts.size());
    for (const auto& contract : state.contracts) {
        auto row = contractRow(contract);
        if (!row.id.empty()) rows.push_back(std::move(row));
    }
    return rows;
}

int rowIndexById(const std::vector<DraftChoiceRow>& rows, const std::string& id)
{
    if (id.empty()) return -1;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].id == id) return static_cast<int>(i);
    }
    return -1;
}

const DraftChoiceRow* rowByIndex(const std::vector<DraftChoiceRow>& rows, int index)
{
    return (index >= 0 && static_cast<std::size_t>(index) < rows.size()) ? &rows[static_cast<std::size_t>(index)] : nullptr;
}

std::string firstExistingText(const DraftChoiceRow& row)
{
    if (!row.display.empty()) return row.display;
    if (!row.name.empty()) return row.name;
    if (!row.type.empty()) return row.type;
    return row.id;
}

bool rowMatchesText(const DraftChoiceRow& row, const std::string& text)
{
    const auto key = normalizeDraftText(text);
    if (key.empty()) return false;

    if (normalizeDraftText(row.name) == key) return true;
    if (normalizeDraftText(row.display) == key) return true;
    if (normalizeDraftText(row.type) == key) return true;
    for (const auto& alias : row.aliases) {
        if (normalizeDraftText(alias) == key) return true;
    }
    return false;
}

const DraftChoiceRow* findRowByText(const std::vector<DraftChoiceRow>& rows, const std::string& text)
{
    for (const auto& row : rows) {
        if (rowMatchesText(row, text)) return &row;
    }
    return nullptr;
}

std::string actorDisplayText(const DraftLinkSelection& selection,
                             const std::vector<DraftChoiceRow>& rows,
                             const DraftSuggestionCandidate* top)
{
    if (!selection.actorId.empty()) {
        const auto index = rowIndexById(rows, selection.actorId);
        const auto* row = rowByIndex(rows, index);
        return row ? firstExistingText(*row) : selection.actorId;
    }
    if (top && !top->label.empty()) return top->label;
    const auto actorText = trim(selection.actorText);
    if (!actorText.empty()) return actorText;
    return {};
}

std::string contractDisplayText(const DraftLinkSelection& selection,
                                const std::vector<DraftChoiceRow>& rows,
                                const DraftSuggestionCandidate* top)
{
    if (!selection.contractId.empty()) {
        const auto index = rowIndexById(rows, selection.contractId);
        const auto* row = rowByIndex(rows, index);
        if (row) {
            if (!row->name.empty()) return row->name;
            return firstExistingText(*row);
        }
        return selection.contractId;
    }
    (void)top;
    return {};
}

std::string actorSeedText(const DraftLinkSelection& selection,
                          const std::string& displayText,
                          const DraftSuggestionCandidate* top)
{
    if (!selection.actorId.empty()) return displayText;
    const auto actorText = trim(selection.actorText);
    if (selection.newActorSelected && actorText.empty()) return {};
    if (!actorText.empty()) return actorText;
    return top ? top->label : std::string{};
}

std::string contractSeedText(const DraftLinkSelection& selection,
                             const std::string& displayText,
                             const DraftSuggestionCandidate* top)
{
    if (!selection.contractId.empty()) return displayText;
    const auto type = trim(selection.type);
    if (selection.newContractSelected && type.empty()) return {};
    if (!type.empty()) return type;
    return {};
}

std::string propertySuggestionSummary(const DraftSuggestionBucket& bucket)
{
    const auto* top = topSuggestion(bucket);
    if (!top) return "No property suggestion";

    std::vector<std::string> labels;
    const auto maxCount = std::min<std::size_t>(2, bucket.candidates.size());
    labels.reserve(maxCount);
    for (std::size_t i = 0; i < maxCount; ++i) {
        if (!bucket.candidates[i].label.empty()) labels.push_back(bucket.candidates[i].label);
    }

    return "Confidence: " + std::to_string(confidencePercent(top)) + "% (" + joinStrings(labels, ", ") + ")";
}

std::vector<std::string> propertyAutoSelectIds(const DraftLinkSelection& selection,
                                               const std::vector<DraftChoiceRow>& properties,
                                               const std::vector<DraftChoiceRow>& contracts,
                                               const std::string& selectedContractId)
{
    std::vector<std::string> ids;
    const std::string source = !trim(selection.propertyText).empty()
        ? selection.propertyText
        : selection.metadata + " " + selection.actorText + " " + selection.type;
    const auto containsNormalized = [](const std::string& haystack, const std::string& needle) {
        const auto h = normalizeDraftText(haystack);
        const auto n = normalizeDraftText(needle);
        return !h.empty() && !n.empty() && h.find(n) != std::string::npos;
    };

    for (const auto& row : properties) {
        if (row.id.empty()) continue;
        if (containsNormalized(source, row.name) || containsNormalized(source, row.display)) {
            appendUnique(ids, row.id);
            continue;
        }
        for (const auto& alias : row.aliases) {
            if (containsNormalized(source, alias)) {
                appendUnique(ids, row.id);
                break;
            }
        }
    }

    for (const auto& suggestion : selection.propertySuggestions.candidates) {
        if (suggestion.confidence < 0.25) continue;
        for (const auto& row : properties) {
            if (row.id.empty()) continue;
            if (row.id == suggestion.entityId || containsNormalized(suggestion.label, row.name) || containsNormalized(suggestion.label, row.display)) {
                appendUnique(ids, row.id);
                break;
            }
        }
    }

    const auto contractId = !selectedContractId.empty() ? selectedContractId : selection.contractId;
    if (!contractId.empty()) {
        const auto contractIndex = rowIndexById(contracts, contractId);
        if (const auto* contractRow = rowByIndex(contracts, contractIndex)) {
            for (const auto& propertyId : contractRow->propertyIds) appendUnique(ids, propertyId);
        }
    }

    return ids;
}

double contractScore(const DraftLinkSelection& selection,
                     const DraftChoiceRow& row,
                     const std::vector<DraftChoiceRow>& actors,
                     const DraftSuggestionCandidate* topContract,
                     const std::string& actorDisplay)
{
    double score = 0.0;
    const auto typeText = trim(selection.type);
    const auto actorText = trim(selection.actorText).empty() ? actorDisplay : trim(selection.actorText);

    if (!typeText.empty()) {
        if (matchesDraftText(row.type, typeText)) score += 600.0;
        if (matchesDraftText(row.name, typeText)) score += 420.0;
        if (matchesDraftText(row.display, typeText)) score += 360.0;
    }

    if (!actorText.empty()) {
        for (const auto& actorId : row.actorIds) {
            const auto actorIndex = rowIndexById(actors, actorId);
            const auto* actorRowPtr = rowByIndex(actors, actorIndex);
            const auto actorName = actorRowPtr ? firstExistingText(*actorRowPtr) : actorId;
            if (matchesDraftText(actorName, actorText)) {
                score += 520.0;
                break;
            }
        }
        if (matchesDraftText(row.name, actorText) || matchesDraftText(row.type, actorText)) score += 180.0;
    }

    for (const auto& propertyId : selection.propertyIds) {
        if (containsId(row.propertyIds, propertyId)) {
            score += 160.0;
            break;
        }
    }

    if (topContract && row.id == topContract->entityId) score += 40.0;
    return score;
}

} // namespace

std::string normalizeDraftText(const std::string& text)
{
    std::string normalized;
    normalized.reserve(text.size());

    bool pendingSpace = false;
    for (const unsigned char byte : text) {
        if (byte >= 128) {
            if (pendingSpace && !normalized.empty()) normalized.push_back(' ');
            pendingSpace = false;
            normalized.push_back(static_cast<char>(byte));
            continue;
        }

        if (std::isalnum(byte)) {
            if (pendingSpace && !normalized.empty()) normalized.push_back(' ');
            pendingSpace = false;
            normalized.push_back(lowerAscii(static_cast<char>(byte)));
        } else {
            pendingSpace = true;
        }
    }

    return trim(normalized);
}

bool matchesDraftText(const std::string& left, const std::string& right)
{
    return normalizeDraftText(left) == normalizeDraftText(right);
}

std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata)
{
    std::vector<std::string> out;
    const std::regex refKeyword("(kund(?:en)?ref|kundenreferenz|referenz|verweis|vertragsnr|vertragsnummer|nr|nummer|ref)", std::regex::icase);
    const std::regex digits("\\b\\d{3,}\\b");

    for (const auto& line : splitLines(metadata)) {
        if (line.empty()) continue;
        if (std::regex_search(line, refKeyword)) appendUnique(out, line);

        for (std::sregex_iterator it(line.begin(), line.end(), digits), end; it != end; ++it) {
            appendUnique(out, it->str());
        }
    }

    return out;
}

DraftTextSignals buildDraftTextSignals(const core::domain::AppState& state,
                                       const ImportedTransaction& transaction)
{
    DraftTextSignals signals;
    const auto metadataText = metadataSignalText(transaction.metadata);
    signals.sharedText = metadataText.empty() ? transaction.metadata : metadataText;

    const auto lines = splitLines(transaction.metadata);
    signals.typeText = extractTypeText(state, signals.sharedText, lines);
    signals.actorText = extractActorText(lines, signals.typeText);
    if (signals.actorText.empty()) signals.actorText = firstMeaningfulLine(transaction.metadata);
    if (signals.actorText.empty()) signals.actorText = leadingPhrase(signals.sharedText, 4);
    signals.propertyText = trim(joinNonEmptyLines({signals.sharedText, signals.actorText, signals.typeText}, " "));
    // Contract text should be composed from type, actor and the original shared metadata
    // to avoid duplicating actor/type which may already be appended to propertyText.
    signals.contractText = trim(joinNonEmptyLines({signals.typeText, signals.actorText, signals.sharedText}, " "));
    return signals;
}

DraftImportSuggestions buildImportSuggestions(const core::domain::AppState& state,
                                              const ImportedTransaction& transaction)
{
    const auto signals = buildDraftTextSignals(state, transaction);

    auto buildBucket = [&](const auto& entities,
                           const std::string& entityType,
                           auto&& labelFn,
                            auto&& textFn,
                            const std::string& sourceText) {
        DraftSuggestionBucket bucket;
        bucket.sourceText = sourceText;

        const auto sourceTokens = tokens(sourceText);
        const auto sourceLeadTokens = tokens(leadingText(sourceText, 4));

        struct RankedSuggestion { DraftSuggestionCandidate value; };
        std::vector<RankedSuggestion> ranked;
        ranked.reserve(entities.size());

        for (const auto& entity : entities) {
            if (!entity) continue;

            DraftSuggestionCandidate suggestion;
            suggestion.entityType = entityType;
            suggestion.entityId = entity->id;
            suggestion.label = labelFn(*entity);
            suggestion.sourceText = sourceText;

            const auto candidateText = normalizeText(textFn(*entity));
            const auto candidateTokens = tokens(textFn(*entity));
            const auto usages = aliasUsages(*entity);

            double score = 0.0;
            std::vector<std::string> matchedAliases;
            double aliasScore = 0.0;
            double recencyScore = 0.0;
            int bestHitCount = 0;
            std::string bestLastUsedAt;

            double strongestUsageFactor = 0.0;
            for (const auto& usage : usages) {
                strongestUsageFactor = std::max(strongestUsageFactor,
                                                aliasHitWeight(usage.hitCount) * 0.65 + aliasRecencyWeight(usage.lastUsedAt) * 0.35);
            }
            if (strongestUsageFactor <= 0.0) strongestUsageFactor = 0.15;

            if (!candidateText.empty() && normalizeDraftText(sourceText).find(candidateText) != std::string::npos) {
                score += 40.0 + (40.0 * strongestUsageFactor * 0.2);
            }
            score += tokenOverlapScore(sourceTokens, candidateTokens) * (12.0 + strongestUsageFactor * 4.0);
            score += tokenOverlapScore(sourceLeadTokens, candidateTokens) * 18.0;

            for (const auto& usage : usages) {
                const std::string alias = usage.alias;
                const auto aliasNorm = normalizeText(alias);
                if (aliasNorm.empty()) continue;

                const double hitWeight = aliasHitWeight(usage.hitCount);
                const double recentWeight = aliasRecencyWeight(usage.lastUsedAt);
                const double usageFactor = 1.0 + hitWeight + recentWeight * 0.75;

                if (normalizeDraftText(sourceText).find(aliasNorm) != std::string::npos) {
                    score += 80.0 * usageFactor;
                    matchedAliases.push_back(alias);
                    aliasScore += hitWeight;
                    recencyScore += recentWeight;
                    bestHitCount = std::max(bestHitCount, usage.hitCount);
                    if (!usage.lastUsedAt.empty() && (bestLastUsedAt.empty() || usage.lastUsedAt > bestLastUsedAt)) bestLastUsedAt = usage.lastUsedAt;
                    continue;
                }

                const auto aliasTokens = tokens(alias);
                const int aliasOverlap = tokenOverlapScore(sourceTokens, aliasTokens);
                if (aliasOverlap > 0) {
                    score += (10.0 * aliasOverlap) * (1.0 + hitWeight * 0.5 + recentWeight * 0.25);
                    matchedAliases.push_back(alias);
                    aliasScore += hitWeight * 0.5;
                    recencyScore += recentWeight * 0.5;
                    bestHitCount = std::max(bestHitCount, usage.hitCount);
                    if (!usage.lastUsedAt.empty() && (bestLastUsedAt.empty() || usage.lastUsedAt > bestLastUsedAt)) bestLastUsedAt = usage.lastUsedAt;
                }
            }

            if (score <= 0.0) {
                score = 0.05;
                suggestion.rationale = "Fallback";
            }

            suggestion.score = score;
            suggestion.confidence = std::clamp(score / 220.0, 0.0, 1.0);
            suggestion.aliasWeight = std::clamp(aliasScore, 0.0, 1.0);
            suggestion.recencyWeight = std::clamp(recencyScore, 0.0, 1.0);
            suggestion.hitCount = bestHitCount;
            suggestion.lastUsedAt = bestLastUsedAt;
            suggestion.matchedAliases = matchedAliases;
            if (!matchedAliases.empty()) {
                suggestion.rationale = "Alias match";
            } else if (!candidateText.empty() && normalizeDraftText(sourceText).find(candidateText) != std::string::npos) {
                suggestion.rationale = "Name match";
            } else {
                suggestion.rationale = "Token overlap";
            }

            ranked.push_back({std::move(suggestion)});
        }

        std::sort(ranked.begin(), ranked.end(), [](const RankedSuggestion& lhs, const RankedSuggestion& rhs) {
            if (lhs.value.score == rhs.value.score) {
                if (lhs.value.confidence == rhs.value.confidence) return lhs.value.label < rhs.value.label;
                return lhs.value.confidence > rhs.value.confidence;
            }
            return lhs.value.score > rhs.value.score;
        });

        constexpr std::size_t kMaxCandidates = 5;
        bucket.candidates.reserve(std::min(kMaxCandidates, ranked.size()));
        for (std::size_t i = 0; i < ranked.size() && i < kMaxCandidates; ++i) {
            bucket.candidates.push_back(std::move(ranked[i].value));
        }

        return bucket;
    };

    const auto actorSourceText = trim(joinNonEmptyLines({signals.actorText, signals.sharedText}, " "));
    const auto& propertySourceText = signals.propertyText;
    const auto contractSourceText = signals.sharedText;

    DraftImportSuggestions suggestions;
    suggestions.actor = buildBucket(state.actors,
                                    "actor",
                                    [](const core::domain::Actor& actor) {
                                        return actor.type.empty() ? actor.name : actor.name + " — " + actor.type;
                                    },
                                    [](const core::domain::Actor& actor) {
                                        std::string text = actor.name;
                                        if (!actor.type.empty()) text += " " + actor.type;
                                        if (!actor.description.empty()) text += " " + actor.description;
                                        return text;
                                    },
                                    actorSourceText);

    suggestions.property = buildBucket(state.properties,
                                      "property",
                                      [](const core::domain::Property& property) {
                                          return property.address.empty() ? property.name : property.name + " — " + property.address;
                                      },
                                      [](const core::domain::Property& property) {
                                          std::string text = property.name;
                                          if (!property.address.empty()) text += " " + property.address;
                                          if (!property.description.empty()) text += " " + property.description;
                                          return text;
                                       },
                                       propertySourceText);

    suggestions.contract = buildBucket(state.contracts,
                                      "contract",
                                      [](const core::domain::Contract& contract) {
                                          if (contract.name.empty()) return contract.type;
                                          if (contract.type.empty()) return contract.name;
                                          return contract.type + " — " + contract.name;
                                      },
                                      [](const core::domain::Contract& contract) {
                                          std::string text = contract.type;
                                          if (!contract.name.empty()) text += " " + contract.name;
                                          if (!contract.description.empty()) text += " " + contract.description;
                                          return text;
                                       },
                                       contractSourceText);

    return suggestions;
}

std::string resolveActorId(const core::domain::AppState& state, const std::string& text)
{
    const auto key = normalizeDraftText(text);
    if (key.empty()) return {};

    for (const auto& actor : state.actors) {
        if (!actor) continue;
        if (normalizeDraftText(actor->name) == key) return actor->id;
        for (const auto& alias : actor->aliases) {
            if (normalizeDraftText(alias) == key) return actor->id;
        }
    }
    return {};
}

std::string resolveContractId(const core::domain::AppState& state, const std::string& text)
{
    const auto key = normalizeDraftText(text);
    if (key.empty()) return {};

    for (const auto& contract : state.contracts) {
        if (!contract) continue;
        if (normalizeDraftText(contract->type) == key) return contract->id;
        if (normalizeDraftText(contract->name) == key) return contract->id;
        for (const auto& alias : contract->aliases) {
            if (normalizeDraftText(alias) == key) return contract->id;
        }
    }
    return {};
}

bool contractIsFullyAllocatable(const core::domain::AppState& state, const std::string& contractId)
{
    if (contractId.empty()) return false;

    bool hasTransactions = false;
    for (const auto& transaction : state.transactions) {
        if (!transaction || transaction->contractId != contractId) continue;
        hasTransactions = true;
        if (!transaction->allocatable) return false;
    }
    return hasTransactions;
}

DraftDerivedState buildDraftDerivedState(const core::domain::AppState& state,
                                         const DraftLinkSelection& selection)
{
    auto effectiveSelection = selection;
    if (trim(effectiveSelection.type).empty() && !trim(effectiveSelection.metadata).empty()) {
        const auto metadataLines = splitLines(effectiveSelection.metadata);
        effectiveSelection.type = extractTypeText(state, effectiveSelection.metadata, metadataLines);
    }

    DraftDerivedState derived;
    const auto actors = actorRows(state);
    const auto properties = propertyRows(state);
    const auto contracts = contractRows(state);

    derived.propertyRows = properties;
    derived.proofSource = proofSourceFor(effectiveSelection.proofImagePath);

    const auto* actorTop = topSuggestion(effectiveSelection.actorSuggestions);
    const auto* propertyTop = topSuggestion(effectiveSelection.propertySuggestions);
    const auto* contractTop = topSuggestion(effectiveSelection.contractSuggestions);
    if (actorTop) {
        derived.actorTopSuggestion = *actorTop;
        derived.hasActorTopSuggestion = true;
    }
    if (propertyTop) {
        derived.propertyTopSuggestion = *propertyTop;
        derived.hasPropertyTopSuggestion = true;
    }
    if (contractTop) {
        derived.contractTopSuggestion = *contractTop;
        derived.hasContractTopSuggestion = true;
    }

    derived.actorDisplayText = actorDisplayText(effectiveSelection, actors, actorTop);
    derived.contractDisplayText = contractDisplayText(effectiveSelection, contracts, contractTop);
    derived.actorSeedText = actorSeedText(effectiveSelection, derived.actorDisplayText, actorTop);
    derived.contractSeedText = contractSeedText(effectiveSelection, derived.contractDisplayText, contractTop);
    derived.propertySuggestionSummary = propertySuggestionSummary(effectiveSelection.propertySuggestions);
    derived.effectiveAllocatable = effectiveSelection.allocatableManualOverride
                                      ? effectiveSelection.allocatable
                                      : (contractIsFullyAllocatable(state, effectiveSelection.contractId) || effectiveSelection.allocatable);

    derived.actorChoices = actors;
    DraftChoiceRow newActor;
    newActor.name = "New Actor";
    newActor.display = "New Actor";
    newActor.synthetic = true;
    newActor.confidence = actorTop ? actorTop->confidence : kSyntheticConfidence;
    newActor.sourceText = derived.actorSeedText;
    derived.actorChoices.insert(derived.actorChoices.begin(), std::move(newActor));

    std::vector<std::pair<double, DraftChoiceRow>> scoredContracts;
    scoredContracts.reserve(contracts.size());
    for (const auto& row : contracts) {
        scoredContracts.push_back({contractScore(effectiveSelection, row, actors, contractTop, derived.actorDisplayText), row});
    }
    std::sort(scoredContracts.begin(), scoredContracts.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first == rhs.first) return lhs.second.display < rhs.second.display;
        return lhs.first > rhs.first;
    });

    DraftChoiceRow newContract;
    newContract.name = "New Contract";
    newContract.display = "New Contract";
    newContract.type = derived.contractSeedText;
    newContract.synthetic = true;
    newContract.confidence = contractTop ? contractTop->confidence : kSyntheticConfidence;
    newContract.sourceText = derived.contractSeedText;
    derived.contractChoices.push_back(std::move(newContract));
    for (const auto& [score, row] : scoredContracts) {
        (void)score;
        derived.contractChoices.push_back(row);
    }

    if (!effectiveSelection.actorId.empty()) {
        derived.actorCurrentIndex = rowIndexById(derived.actorChoices, effectiveSelection.actorId);
    } else if (effectiveSelection.newActorSelected) {
        derived.actorCurrentIndex = 0;
    } else if (actorTop && !actorTop->entityId.empty() && actorTop->confidence >= 0.2) {
        derived.actorCurrentIndex = rowIndexById(derived.actorChoices, actorTop->entityId);
    } else if (!derived.actorSeedText.empty()) {
        if (const auto* match = findRowByText(actors, derived.actorSeedText)) {
            derived.actorCurrentIndex = rowIndexById(derived.actorChoices, match->id);
        }
    }
    if (derived.actorCurrentIndex < 0) derived.actorCurrentIndex = derived.actorChoices.empty() ? -1 : 0;

    if (!effectiveSelection.contractId.empty()) {
        derived.contractCurrentIndex = rowIndexById(derived.contractChoices, effectiveSelection.contractId);
    } else if (effectiveSelection.newContractSelected) {
        derived.contractCurrentIndex = 0;
    } else if (!scoredContracts.empty() && scoredContracts.front().first >= 900.0) {
        derived.contractCurrentIndex = rowIndexById(derived.contractChoices, scoredContracts.front().second.id);
    } else if (!derived.contractSeedText.empty()
               && (!trim(effectiveSelection.actorText).empty()
                   || !effectiveSelection.actorId.empty()
                   || !effectiveSelection.propertyIds.empty())) {
        if (const auto* match = findRowByText(contracts, derived.contractSeedText)) {
            derived.contractCurrentIndex = rowIndexById(derived.contractChoices, match->id);
        }
    }
    if (derived.contractCurrentIndex < 0) derived.contractCurrentIndex = derived.contractChoices.empty() ? -1 : 0;

    std::string selectedContractId = effectiveSelection.contractId;
    if (selectedContractId.empty() && derived.contractCurrentIndex > 0) {
        if (const auto* contractRow = rowByIndex(derived.contractChoices, derived.contractCurrentIndex);
            contractRow && !contractRow->synthetic) {
            selectedContractId = contractRow->id;
        }
    }
    derived.autoPropertyIds = propertyAutoSelectIds(effectiveSelection, properties, contracts, selectedContractId);

    return derived;
}

} // namespace core::importing
