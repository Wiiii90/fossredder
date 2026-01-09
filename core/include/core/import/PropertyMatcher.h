#pragma once

#include "core/import/PropertyMatch.h"

#include <memory>
#include <string>
#include <vector>

class Property;

class PropertyMatcher {
public:
    PropertyMatch match(const std::string& text, const std::vector<std::shared_ptr<Property>>& properties) const;

private:
    static std::string normalize(const std::string& s);
    static bool containsNormalized(const std::string& haystackNorm, const std::string& needleNorm);
};
