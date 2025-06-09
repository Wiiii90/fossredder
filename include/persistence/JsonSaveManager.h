#pragma once
#include "persistence/ISaveManager.h"

class JsonSaveManager : public ISaveManager {
public:
    bool saveToFile(const std::string& path, const AppState& state) override;
    std::optional<AppState> loadFromFile(const std::string& path) override;
};