#include "core/pch.h"
#include "core/import/PropertyMatcher.h"

#include "core/models/Property.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

static std::string trimSpacesProp(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string PropertyMatcher::normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    bool prevSpace = false;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '_') {
            out.push_back(static_cast<char>(std::tolower(c)));
            prevSpace = false;
            continue;
        }

        if (std::isspace(c) || c == '-' || c == '/' || c == '.' || c == ',' || c == ':' || c == ';' || c == '(' || c == ')' || c == '\\') {
            if (!out.empty() && !prevSpace) {
                out.push_back(' ');
                prevSpace = true;
            }
            continue;
        }
    }

    return trimSpacesProp(out);
}

bool PropertyMatcher::containsNormalized(const std::string& haystackNorm, const std::string& needleNorm) {
    if (needleNorm.empty()) return false;
    if (haystackNorm.size() < needleNorm.size()) return false;

    auto pos = haystackNorm.find(needleNorm);
    if (pos == std::string::npos) return false;

    bool leftOk = (pos == 0) || (haystackNorm[pos - 1] == ' ');
    bool rightOk = (pos + needleNorm.size() == haystackNorm.size()) || (haystackNorm[pos + needleNorm.size()] == ' ');
    return leftOk && rightOk;
}

PropertyMatch PropertyMatcher::match(const std::string& text, const std::vector<std::shared_ptr<Property>>& properties) const {
    PropertyMatch out;
    if (properties.empty()) return out;

    const std::string tnorm = normalize(text);
    if (tnorm.empty()) return out;

    struct Scored {
        std::string propertyId;
        double score = 0.0;
        std::string mention;
        std::string evidence;
    };

    std::vector<Scored> scored;
    scored.reserve(properties.size());

    for (const auto& p : properties) {
        if (!p) continue;
        if (p->id.empty()) continue;

        Scored s;
        s.propertyId = p->id;

        const std::string nameNorm = normalize(p->name);
        if (!nameNorm.empty()) {
            if (tnorm == nameNorm) {
                s.score = std::max(s.score, 2.0);
                s.mention = p->name;
                s.evidence = "name_exact";
            } else if (containsNormalized(tnorm, nameNorm)) {
                s.score = std::max(s.score, 1.2);
                s.mention = p->name;
                s.evidence = "name";
            }
        }

        const std::string addrNorm = normalize(p->address);
        if (!addrNorm.empty() && containsNormalized(tnorm, addrNorm)) {
            // address match is slightly weaker than exact name
            s.score = std::max(s.score, 1.1);
            if (s.mention.empty()) s.mention = p->address;
            if (s.evidence.empty()) s.evidence = "address";
        }

        // also allow short name fragments: e.g., "foss" matches "fossredder"
        if (!nameNorm.empty()) {
            for (size_t len = 3; len <= 6 && len < nameNorm.size(); ++len) {
                // check if a short fragment of the property name is present
                std::string frag = nameNorm.substr(0, len);
                if (frag.size() < 3) continue;
                if (containsNormalized(tnorm, frag)) {
                    double base = 0.9;
                    if (frag.size() <= 4) base = 0.8;
                    s.score = std::max(s.score, base);
                    if (s.mention.empty()) s.mention = p->name;
                    if (s.evidence.empty()) s.evidence = "name_fragment";
                }
            }
        }

        if (s.score > 0.0) scored.push_back(std::move(s));
    }

    if (scored.empty()) return out;

    std::sort(scored.begin(), scored.end(), [](const Scored& a, const Scored& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.propertyId < b.propertyId;
    });

    out.candidates.reserve(scored.size());
    for (const auto& s : scored) out.candidates.emplace_back(s.propertyId, s.score);

    const auto& best = scored.front();
    const double bestScore = best.score;
    const double secondScore = (scored.size() >= 2) ? scored[1].score : 0.0;

    const double threshold = 1.05;
    const double margin = 0.04;

    out.confidence = bestScore;
    out.mention = best.mention;
    out.evidence = best.evidence;

    if (bestScore >= threshold && (bestScore - secondScore) >= margin) {
        out.propertyId = best.propertyId;
    }

    return out;
}
