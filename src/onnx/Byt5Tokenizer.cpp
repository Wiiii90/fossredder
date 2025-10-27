#include "onnx/Byt5Tokenizer.h"

// Implementierung für das ITextTokenizer-Interface (ohne Prefix)
std::vector<int64_t> Byt5Tokenizer::encode(const std::string& text) const {
    std::vector<int64_t> out;
    for (unsigned char c : text) {
        out.push_back(static_cast<int64_t>(c) + Byt5Vocab::byte_offset);
    }
    out.push_back(Byt5Vocab::eos_id);
    return out;
}

// Optionale Methode für Prefix-Support (nicht im Interface)
std::vector<int64_t> Byt5Tokenizer::encode(const std::string& text, const std::string& prefix) const {
    std::vector<int64_t> out;
    // optionales Prompt/Prefix (z.B. Task-Hinweis)
    if (!prefix.empty()) {
        for (unsigned char c : prefix) {
            out.push_back(static_cast<int64_t>(c) + Byt5Vocab::byte_offset);
        }
    }
    for (unsigned char c : text) {
        out.push_back(static_cast<int64_t>(c) + Byt5Vocab::byte_offset);
    }
    out.push_back(Byt5Vocab::eos_id);
    return out;
}

std::string Byt5Tokenizer::decode(const std::vector<int64_t>& ids) const {
    std::string out;
    for (auto id : ids) {
        if (id == Byt5Vocab::eos_id) break;
        if (id >= Byt5Vocab::byte_offset && id <= Byt5Vocab::byte_offset + 255) {
            unsigned char byte = static_cast<unsigned char>(id - Byt5Vocab::byte_offset);
            out.push_back(static_cast<char>(byte));
        }
        // pad/unk werden ignoriert
    }
    return out;
}
