#pragma once
#include <string>
#include <optional>

// Beispielhafte Felder für die Konfiguration
struct Config {
    std::string ocrLanguage = "deu";
    std::string bankTemplate = "default";
    int ocrDpi = 300;

    // ONNX model paths (set via .env or UI)
    std::string byt5_encoder;
    std::string byt5_decoder_init;
    std::string byt5_decoder;
};