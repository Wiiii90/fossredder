#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Gemeinsames Interface für alle Text-Tokenizer (z.B. FlanT5, ByT5)
class ITextTokenizer {
public:
    virtual ~ITextTokenizer() = default;
    virtual std::vector<int64_t> encode(const std::string& text) const = 0;
    virtual std::string decode(const std::vector<int64_t>& ids) const = 0;
    virtual int64_t pad_id() const = 0;
    virtual int64_t eos_id() const = 0;
    virtual void setDebug(bool enabled) = 0;
};
