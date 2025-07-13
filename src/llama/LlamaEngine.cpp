#include "llama/LlamaEngine.h"
#include "models/Transaction.h"
#include "llama.h"
#include <map>
#include <string>
#include <regex>
#include <memory>
#include <stdexcept>

LlamaEngine::LlamaEngine(const std::string& modelPath)
    : model_(nullptr), ctx_(nullptr), modelPath_(modelPath)
{
    model_ = llama_load_model_from_file(modelPath_.c_str(), llama_model_default_params());
    if (!model_) {
        throw std::runtime_error("Failed to load Llama model: " + modelPath_);
    }
    ctx_ = llama_new_context_with_model(model_, llama_context_default_params());
    if (!ctx_) {
        llama_free_model(model_);
        throw std::runtime_error("Failed to create Llama context with model: " + modelPath_);
    }
}

LlamaEngine::~LlamaEngine() {
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
}

static std::string runLlamaPrompt(llama_context* ctx, const std::string& prompt) {
    if (!ctx) throw std::runtime_error("Llama context is not initialized!");

    const llama_model* model = llama_get_model(ctx);
    const llama_vocab* vocab = llama_model_get_vocab(model);

    // Tokenize prompt
    std::vector<llama_token> tokens(prompt.size() + 8);
    int n_tokens = llama_tokenize(
        vocab,
        prompt.c_str(),
        static_cast<int32_t>(prompt.size()),
        tokens.data(),
        static_cast<int32_t>(tokens.size()),
        true,
        false
    );
    if (n_tokens < 0) n_tokens = -n_tokens;
    tokens.resize(n_tokens);

    // Prepare batch
    llama_batch batch = llama_batch_init(static_cast<int32_t>(tokens.size()), 0, 1);
    for (int i = 0; i < n_tokens; ++i) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.seq_id[i] = 0;
        batch.n_tokens++;
    }

    int ret = llama_decode(ctx, batch);
    if (ret != 0) {
        throw std::runtime_error("llama_decode failed, ret = " + std::to_string(ret));
    }

    // Greedy decoding
    std::string output;
    int n_vocab = llama_vocab_n_tokens(vocab);
    llama_token eos_token = llama_vocab_eos(vocab);

    char piece_buf[512];
    for (int step = 0; step < 256; ++step) {
        const float* logits = llama_get_logits(ctx);
        llama_token next_token = static_cast<llama_token>(std::max_element(logits, logits + n_vocab) - logits);
        if (next_token == eos_token) break;

        // Token to text
        int n = llama_token_to_piece(vocab, next_token, piece_buf, sizeof(piece_buf), 0, true);
        if (n > 0) output.append(piece_buf, n);

        // Feed token back for next step
        llama_batch next_batch = llama_batch_init(1, 0, 1);
        next_batch.token[0] = next_token;
        next_batch.pos[0] = step + n_tokens;
        next_batch.seq_id[0] = 0;
        next_batch.n_tokens = 1;
        int ret2 = llama_decode(ctx, next_batch);
        if (ret2 != 0) break;
    }
    return output;
}

static std::map<std::string, std::string> parseLlamaOutput(const std::string& llamaOutput) {
    std::map<std::string, std::string> fields;
    std::regex re(R"(([\w\s]+):\s*([^\n]+))");
    std::smatch match;
    std::string::const_iterator searchStart(llamaOutput.cbegin());
    while (std::regex_search(searchStart, llamaOutput.cend(), match, re)) {
        std::string key = match[1].str();
        std::string value = match[2].str();
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        fields[key] = value;
        searchStart = match.suffix().first;
    }
    return fields;
}

void LlamaEngine::enrichTransactions(std::vector<std::shared_ptr<Transaction>>& transactions) {
    for (auto& tx : transactions) {
        std::string prompt =
            "Extract all key-value fields from the following transaction details. "
            "Return the result as lines in the format 'Field: Value'.\n\nDetails:\n" + tx->details;
        std::string llamaOutput = runLlamaPrompt(ctx_, prompt);
        tx->extractedFields = parseLlamaOutput(llamaOutput);
    }
}