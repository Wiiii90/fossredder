#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "onnx/ITextTokenizer.h"

// Minimal FlanT5 Tokenizer (JSON-based, for demonstration)
class FlanT5Tokenizer : public ITextTokenizer {
public:
    FlanT5Tokenizer(const std::string& tokenizer_json_path);
    std::vector<int64_t> encode(const std::string& text) const override;
    std::string decode(const std::vector<int64_t>& ids) const override;
    int64_t pad_id() const override { return pad_id_; }
    int64_t eos_id() const override { return eos_id_; }
    void setDebug(bool enabled) override { debug_ = enabled; }
    // Helper to get token string for a given ID
    std::string token_for_id(int64_t id) const {
        auto it = id_to_token_.find(id);
        if (it != id_to_token_.end()) return it->second;
        return "<unk>";
    }
private:
    std::unordered_map<std::string, int64_t> token_to_id_;
    std::unordered_map<int64_t, std::string> id_to_token_;
    int64_t pad_id_ = 0;
    int64_t eos_id_ = 1;
    bool debug_ = false;
    void load_tokenizer(const std::string& path);
};
