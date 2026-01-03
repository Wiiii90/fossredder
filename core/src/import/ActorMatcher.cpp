#include "core/import/ActorMatcher.h"

#include "core/models/Actor.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

static std::string trimSpaces(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string ActorMatcher::normalize(const std::string& s) {
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

    return trimSpaces(out);
}

bool ActorMatcher::containsNormalized(const std::string& haystackNorm, const std::string& needleNorm) {
    if (needleNorm.empty()) return false;
    if (haystackNorm.size() < needleNorm.size()) return false;

    auto pos = haystackNorm.find(needleNorm);
    if (pos == std::string::npos) return false;

    bool leftOk = (pos == 0) || (haystackNorm[pos - 1] == ' ');
    bool rightOk = (pos + needleNorm.size() == haystackNorm.size()) || (haystackNorm[pos + needleNorm.size()] == ' ');
    return leftOk && rightOk;
}

ActorMatch ActorMatcher::match(const std::string& text, const std::vector<std::shared_ptr<Actor>>& actors) const {
    ActorMatch out;
    if (actors.empty()) return out;

    const std::string tnorm = normalize(text);
    if (tnorm.empty()) return out;

    struct Scored {
        std::string actorId;
        double score = 0.0;
        std::string mention;
        std::string evidence;
    };

    std::vector<Scored> scored;
    scored.reserve(actors.size());

    for (const auto& a : actors) {
        if (!a) continue;
        if (a->id.empty()) continue;

        Scored s;
        s.actorId = a->id;

        const std::string nameNorm = normalize(a->name);
        if (!nameNorm.empty()) {
            if (tnorm == nameNorm) {
                s.score = std::max(s.score, 2.0);
                s.mention = a->name;
                s.evidence = "name_exact";
            } else if (containsNormalized(tnorm, nameNorm)) {
                s.score = std::max(s.score, 1.2);
                s.mention = a->name;
                s.evidence = "name";
            }
        }

        for (const auto& alias : a->aliases) {
            const std::string aliasNorm = normalize(alias);
            if (aliasNorm.size() < 2) continue;
            if (!containsNormalized(tnorm, aliasNorm)) continue;

            double base = 1.2;
            if (aliasNorm.size() <= 4) base = 1.05;

            if (base > s.score) {
                s.score = base;
                s.mention = alias;
                s.evidence = "alias";
            }
        }

        if (s.score > 0.0) scored.push_back(std::move(s));
    }

    if (scored.empty()) return out;

    std::sort(scored.begin(), scored.end(), [](const Scored& a, const Scored& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.actorId < b.actorId;
    });

    out.candidates.reserve(scored.size());
    for (const auto& s : scored) out.candidates.emplace_back(s.actorId, s.score);

    const auto& best = scored.front();
    const double bestScore = best.score;
    const double secondScore = (scored.size() >= 2) ? scored[1].score : 0.0;

    const double threshold = 1.1;
    const double margin = 0.05;

    out.confidence = bestScore;
    out.mention = best.mention;
    out.evidence = best.evidence;

    if (bestScore >= threshold && (bestScore - secondScore) >= margin) {
        out.actorId = best.actorId;
    }

    return out;
}