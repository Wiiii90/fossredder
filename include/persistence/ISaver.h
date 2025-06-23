#pragma once
#include <string>
#include <optional>

class AppState;

class ISaver {
public:
    virtual ~ISaver() = default;
    virtual bool saveToFile(const std::string& path, const AppState& state) = 0;
    virtual std::optional<AppState> loadFromFile(const std::string& path) = 0;
};