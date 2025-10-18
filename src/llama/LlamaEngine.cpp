#include "llama/LlamaEngine.h"
#include "llama/prompts/Prompts.h"
#include "models/Transaction.h"
#include "llama.h"
#include "util/OutputParser.h"
#include <QDebug>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <algorithm>

llama_sampler* sampler_ = nullptr;

LlamaEngine::LlamaEngine(const std::string& modelPath)
	: model_(nullptr), ctx_(nullptr), modelPath_(modelPath) {
	llama_backend_init();
	model_ = llama_load_model_from_file(modelPath.c_str(), llama_model_default_params());
	if (!model_) throw std::runtime_error("Failed to load model: " + modelPath);

	llama_context_params params = llama_context_default_params();
	params.n_ctx = 4096;
	params.n_threads = 8;
	ctx_ = llama_new_context_with_model(model_, params);
	if (!ctx_) throw std::runtime_error("Failed to create context with model: " + modelPath);

	llama_memory_clear(llama_get_memory(ctx_), false);

	llama_sampler_chain_params sparams = llama_sampler_chain_default_params();
	sampler_ = llama_sampler_chain_init(sparams);
	llama_sampler_chain_add(sampler_, llama_sampler_init_top_k(60));
	llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(0.95f, 1));
	llama_sampler_chain_add(sampler_, llama_sampler_init_temp(0.9f));
	llama_sampler_chain_add(sampler_, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

	qDebug() << "LlamaEngine: loaded model and context from" << QString::fromStdString(modelPath);
	qDebug() << "LlamaEngine: n_ctx=" << params.n_ctx << ", n_threads=" << params.n_threads;
}

LlamaEngine::~LlamaEngine() {
	if (sampler_) llama_sampler_free(sampler_);
	if (ctx_) llama_free(ctx_);
	if (model_) llama_free_model(model_);
	llama_backend_free();
}

std::string LlamaEngine::buildPrompt(const std::string& system, const std::string& user) {
	return "<|im_start|>system\n" + system + "<|im_end|>\n" +
		"<|im_start|>user\n" + user + "<|im_end|>\n" +
		"<|im_start|>assistant\n";
}

std::string LlamaEngine::runPrompt(const std::string& prompt, int maxTokens,
	int top_k, float top_p, float temperature, float repeat_penalty) {
	if (!ctx_ || prompt.empty()) return "";
	const llama_model* model = llama_get_model(ctx_);
	const llama_vocab* vocab = llama_model_get_vocab(model);
	std::vector<llama_token> prompt_tokens(prompt.size() + 32);
	int n_prompt = llama_tokenize(vocab, prompt.c_str(), (int)prompt.size(), prompt_tokens.data(), (int)prompt_tokens.size(), false, true);
	if (n_prompt <= 0) return "";
	prompt_tokens.resize(n_prompt);
	llama_batch batch = llama_batch_init(n_prompt, 0, 1);
	for (int i = 0; i < n_prompt; ++i) {
		batch.token[i] = prompt_tokens[i];
		batch.pos[i] = i;
		batch.n_seq_id[i] = 1;
		batch.seq_id[i][0] = 0;
		batch.logits[i] = (i == n_prompt - 1) ? 1 : 0;
	}
	batch.n_tokens = n_prompt;
	llama_decode(ctx_, batch);
	llama_batch_free(batch);
	std::vector<llama_token> generated;
	llama_token eos = llama_vocab_eos(vocab);
	const std::string stop_token = "<|im_end|>";
	for (int i = 0; i < maxTokens; ++i) {
		llama_token next = llama_sampler_sample(sampler_, ctx_, -1);
		llama_sampler_accept(sampler_, next);
		if (next == eos) break;
		char token_buf[64];
		int len = llama_detokenize(vocab, &next, 1, token_buf, sizeof(token_buf), true, true);
		std::string token_str = (len > 0) ? std::string(token_buf, len) : "";
		if (!token_str.empty() && token_str.find(stop_token) != std::string::npos)
			break;
		generated.push_back(next);
		llama_batch next_batch = llama_batch_init(1, 0, 1);
		next_batch.token[0] = next;
		next_batch.pos[0] = n_prompt + generated.size() - 1;
		next_batch.n_seq_id[0] = 1;
		next_batch.seq_id[0][0] = 0;
		next_batch.n_tokens = 1;
		next_batch.logits[0] = 1;
		llama_decode(ctx_, next_batch);
		llama_batch_free(next_batch);
	}
	std::vector<char> out_buf(8192);
	int out_len = llama_detokenize(vocab, generated.data(), (int)generated.size(), out_buf.data(), out_buf.size(), true, true);
	return (out_len > 0) ? std::string(out_buf.data(), out_len) : "";
}

void LlamaEngine::resetContext() {
	if (ctx_) {
		llama_memory_clear(llama_get_memory(ctx_), false);
	}
}
