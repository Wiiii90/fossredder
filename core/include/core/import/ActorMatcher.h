#pragma once

#include "core/import/ActorMatch.h"

#include <memory>
#include <string>
#include <vector>

class Actor;

class ActorMatcher {
public:
    ActorMatch match(const std::string& text, const std::vector<std::shared_ptr<Actor>>& actors) const;

private:
    static std::string normalize(const std::string& s);
    static bool containsNormalized(const std::string& haystackNorm, const std::string& needleNorm);
};
