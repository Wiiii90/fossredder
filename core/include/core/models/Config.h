#pragma once

#include "core/constants/CoreDefaults.h"

#include <string>

namespace core::domain {

enum class Language { EN, DE, FR };

struct Config {
    Language language = Language::DE;

    std::string languageToString() const {
        switch (language) {
            case Language::EN: return std::string(core::constants::config::languages::kEnglish);
            case Language::FR: return std::string(core::constants::config::languages::kFrench);
            default: return std::string(core::constants::config::languages::kGerman);
        }
    }

    static Language languageFromString(const std::string& s) {
        if (s == core::constants::config::languages::kEnglish) return Language::EN;
        if (s == core::constants::config::languages::kFrench) return Language::FR;
        return Language::DE;
    }
};

}
