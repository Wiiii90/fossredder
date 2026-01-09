#pragma once

#include <string>
#include <utility>
#include <vector>

struct PropertyMatch {
    // matched property id if confident enough
    std::string propertyId;
    // candidate property ids with scores
    std::vector<std::pair<std::string, double>> candidates;
    // the text snippet that was matched (name/address)
    std::string mention;
    // evidence type like "name", "address", "alias"
    std::string evidence;
    // confidence score of best match
    double confidence = 0.0;

    bool hasMatch() const noexcept { return !propertyId.empty(); }
};
