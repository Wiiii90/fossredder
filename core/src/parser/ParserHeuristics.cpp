#include "core/parser/ParserHeuristics.h"
#include "core/utils/Util.h"

#include <vector>
#include <regex>

using utils::lowerAscii;
using utils::trim;

namespace core::parser::heuristics {

// Header heuristics
bool isHeaderNoiseLine(const std::string& line) {
    const auto l = lowerAscii(line);
    if (l.size() <= 2) return true;

    const auto toks = utils::splitWhitespace(l);
    if (toks.empty()) return false;

    std::vector<std::string> ntoks; ntoks.reserve(toks.size());
    for (const auto& t : toks) ntoks.push_back([](const std::string& s){
        std::string o;
        o.reserve(s.size());
        for (unsigned char c : utils::lowerAscii(s)) {
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) o.push_back(static_cast<char>(c));
        }
        return o;
    }(t));

    const std::vector<std::string> noise = {
        "kontoauszug",
        "auszugnr",
        "seitenr",
        "ustidnr",
        "ansprechpartner",
        "telefonnummer",
        "kontowahrung",
    };

    int matches = 0;
    for (const auto& n : noise) {
        for (const auto& t : ntoks) {
            if (t == n) { ++matches; break; }
        }
        if (matches >= 2) return true;
    }

    return false;
}

// Footer heuristics
static bool containsAny(const std::string& hayLower, std::initializer_list<const char*> needlesLower) {
    for (auto n : needlesLower) {
        if (hayLower.find(n) != std::string::npos) return true;
    }
    return false;
}

bool isFooterLine(const std::string& line) {
    const auto l = lowerAscii(line);
    return containsAny(l, { "folgeseite", "vsa000", "fil200", "ktea", "folgende seite" });
}

bool isPostTransactionFootnote(const std::string& line) {
    const auto l = lowerAscii(line);
    try {
        static const std::regex amountDetect(R"(\d+[\.,]\d{2})");
        if (std::regex_search(l, amountDetect)) return false; // prefer amount-bearing lines for parsing
    } catch(...) {}

    const std::initializer_list<const char*> strong = {
        "guthaben sind als",
        "einlagensicherungsgesetzes",
        "informationsbogen",
        "der angegebene kontostand",
        "wertstellung der buchungen",
        "kontoüberziehung",
        "neuer kontostand",
        "alter kontostand",
        "folgeseite",
    };
    for (auto p : strong) if (l.find(p) != std::string::npos) return true;

    int weakMatches = 0;
    if (l.find("kontostand") != std::string::npos) ++weakMatches;
    if (l.find("guthaben") != std::string::npos) ++weakMatches;
    if (l.find("einlagen") != std::string::npos) ++weakMatches;
    if (l.find("betrag") != std::string::npos) ++weakMatches;
    return weakMatches >= 2;
}

// Transaction heuristics
bool isTransactionsSectionHeader(const std::string& line) {
    const auto l = lowerAscii(line);
    if (l.find("angaben zu den umsätzen") != std::string::npos) return true;
    const bool hasValuta = (l.find("valuta") != std::string::npos);
    const bool hasUmsaetze = (l.find("umsätzen") != std::string::npos) || (l.find("umsaetzen") != std::string::npos);
    const bool hasAngaben = (l.find("angaben") != std::string::npos);
    return hasValuta && (hasUmsaetze || hasAngaben);
}

bool isDebitCreditHeaderLine(const std::string& line) {
    std::string n;
    n.reserve(line.size());
    for (unsigned char c : utils::lowerAscii(line)) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) n.push_back(static_cast<char>(c));
    }
    return (n.find("zuihrenlasten") != std::string::npos) && (n.find("zuihrengunsten") != std::string::npos);
}

} // namespace core::parser::heuristics
