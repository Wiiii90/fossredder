#pragma once

#include <string>
#include <utility>
#include <vector>

struct ActorMatch {
    std::string actorId;
    std::vector<std::pair<std::string, double>> candidates;
    std::string mention;
    std::string evidence;
    double confidence = 0.0;

    bool hasMatch() const noexcept { return !actorId.empty(); }
};
