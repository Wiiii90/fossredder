#pragma once

#include <string>

enum class Language { EN, DE, FR };

struct Config {
    Language language = Language::DE;
    std::string languageToString() const {
        switch (language) {
            case Language::EN: return "EN";
            case Language::FR: return "FR";
            default: return "DE";
        }
    }
    static Language languageFromString(const std::string& s) {
        if (s == "EN") return Language::EN;
        if (s == "FR") return Language::FR;
        return Language::DE;
    }
};