#pragma once
#include "llama/ILlamaEngine.h"
#include "llama.h"
#include <memory>
#include <string>

class LlamaEngine : public ILlamaEngine {
public:
    explicit LlamaEngine(const std::string& modelPath);
    ~LlamaEngine() override;

    std::string runPrompt(const std::string& prompt, int maxTokens = 128,
        int top_k = 40, float top_p = 0.9f, float temperature = 0.7f, float repeat_penalty = 1.1f);
    void resetContext(); // Kontext/KV-Cache zurücksetzen

private:
    llama_model* model_;
    llama_context* ctx_;
    std::string modelPath_;
};