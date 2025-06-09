#pragma once
#include <string>
#include <optional>
#include <vector>
#include <memory>

class Config;

class IConfigManager {
public:
    virtual ~IConfigManager() = default;

    // Speichert die übergebene Konfiguration unter dem gegebenen Namen
    virtual bool saveConfig(const std::string& name, const std::shared_ptr<Config>& config) = 0;

    // Lädt eine Konfiguration anhand des Namens
    virtual std::optional<std::shared_ptr<Config>> loadConfig(const std::string& name) const = 0;

    // Gibt alle gespeicherten Konfigurationsnamen zurück
    virtual std::vector<std::string> listConfigs() const = 0;

    // Löscht eine gespeicherte Konfiguration
    virtual bool deleteConfig(const std::string& name) = 0;

    // Setzt die Default-Konfiguration
    virtual void setDefaultConfig(const std::string& name) = 0;

    // Gibt die aktuell gesetzte Default-Konfiguration zurück
    virtual std::optional<std::shared_ptr<Config>> getDefaultConfig() const = 0;

    // Gibt den Namen der Default-Konfiguration zurück
    virtual std::string getDefaultConfigName() const = 0;
};