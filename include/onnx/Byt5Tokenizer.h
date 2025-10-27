#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "onnx/ITextTokenizer.h"

// Minimaler Byte-Tokenizer für ByT5-small
// Annahmen (ByT5-Standard):
//  - pad_id = 0, eos_id = 1, unk_id = 2
//  - Bytes werden auf Token-IDs [3..258] gemappt: id = byte + 3
struct Byt5Vocab {
    static constexpr int32_t pad_id = 0;
    static constexpr int32_t eos_id = 1;
    static constexpr int32_t unk_id = 2;
    static constexpr int32_t byte_offset = 3; // id = byte + 3
};

class Byt5Tokenizer : public ITextTokenizer {
public:
    // UTF-8 -> Bytes -> Token-IDs (prefix optional)
    std::vector<int64_t> encode(const std::string& text) const override;
    std::vector<int64_t> encode(const std::string& text, const std::string& prefix) const;
    std::string decode(const std::vector<int64_t>& ids) const override;
    int64_t pad_id() const override { return Byt5Vocab::pad_id; }
    int64_t eos_id() const override { return Byt5Vocab::eos_id; }
    void setDebug(bool enabled) override { debug_ = enabled; }
private:
    bool debug_ = false;
};
