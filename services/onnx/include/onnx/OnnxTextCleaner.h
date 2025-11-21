#pragma once
#include "onnx/ITextCleaner.h"
#include "onnx/ITextTokenizer.h"
#include <string>
#include <memory>
#include <vector>
#include <onnxruntime_cxx_api.h>

class OnnxTextCleaner : public ITextCleaner {
public:
    struct ModelPaths {
        std::string encoder;
        std::string decoder;
        std::string init_decoder;
        std::string tokenizer_json; // optional if using HF tokenizers
        std::string sp_model;       // optional (SentencePiece for mT5)
        std::string tokenizer_type; // "flant5" oder "byt5"
    };

    explicit OnnxTextCleaner(const ModelPaths& paths);
    ~OnnxTextCleaner();

    std::string clean(const std::string& input) override;

    // Optional helpers (not part of interface)
    double lastMeanConfidence() const { return last_mean_conf_; }

private:
    Ort::Env env_;
    Ort::SessionOptions opts_;
    std::unique_ptr<Ort::Session> enc_;
    std::unique_ptr<Ort::Session> dec_init_;
    std::unique_ptr<Ort::Session> dec_;
    ModelPaths paths_;
    std::unique_ptr<ITextTokenizer> tokenizer_;
    bool debug_ = false;
    int debug_topk_ = 3;
    int max_len_default_ = 256;
    double last_mean_conf_ = 0.0;
    std::string greedy_decode_t5(const std::string& input, int max_len = 256);
};
