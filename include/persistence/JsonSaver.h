#pragma once
#include "persistence/ISaver.h"

class JsonSaver : public ISaver {
public:
    bool saveToFile(const std::string& path, const AppState& state) override;
    std::optional<AppState> loadFromFile(const std::string& path) override;
};