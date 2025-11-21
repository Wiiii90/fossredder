#include "onnx/OnnxTextCleaner.h"
#include "onnx/FlanT5Tokenizer.h"
#include "onnx/Byt5Tokenizer.h"
#include <stdexcept>
#include <array>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>
#include <QDebug>
#include "utils/Env.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
#ifdef _WIN32
    static std::wstring utf8_to_wstring(const std::string& s) {
        if (s.empty()) return {};
        int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
        if (len <= 0) return {};
        std::wstring w;
        w.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), len);
        return w;
    }
#endif

    static std::basic_string<ORTCHAR_T> to_ort_path(const std::string& s) {
#ifdef _WIN32
        return utf8_to_wstring(s);
#else
        return s;
#endif
    }
}

OnnxTextCleaner::OnnxTextCleaner(const ModelPaths& paths)
    : env_(ORT_LOGGING_LEVEL_WARNING, "onnx-cleaner"), paths_(paths) {
    debug_ = !env::get("ONNX_CLEANER_DEBUG", "").empty();

    // Tokenizer-Auswahl
    if (paths.tokenizer_type == "byt5") {
        tokenizer_ = std::make_unique<Byt5Tokenizer>();
        qDebug() << "[OnnxTextCleaner] Using Byt5Tokenizer for tokenization.";
    } else {
        // Default: FlanT5
        tokenizer_ = std::make_unique<FlanT5Tokenizer>(paths.tokenizer_json);
        qDebug() << "[OnnxTextCleaner] Using FlanT5Tokenizer for tokenization.";
    }
    tokenizer_->setDebug(debug_);

    if (!paths.encoder.empty()) {
        auto enc_path = to_ort_path(paths.encoder);
        enc_ = std::make_unique<Ort::Session>(env_, enc_path.c_str(), opts_);
    }
    if (!paths.init_decoder.empty()) {
        auto init_dec_path = to_ort_path(paths.init_decoder);
        dec_init_ = std::make_unique<Ort::Session>(env_, init_dec_path.c_str(), opts_);
    }
    if (!paths.decoder.empty()) {
        auto dec_path = to_ort_path(paths.decoder);
        dec_ = std::make_unique<Ort::Session>(env_, dec_path.c_str(), opts_);
    }
}

OnnxTextCleaner::~OnnxTextCleaner() = default;

std::string OnnxTextCleaner::clean(const std::string& input) {
    qDebug() << "[OnnxTextCleaner] clean() called with input:" << QString::fromStdString(input);
    try {
        const int max_len = 128;
        std::vector<int64_t> src_ids = tokenizer_->encode(input);
        QStringList idStrings;
        for (auto id : src_ids) idStrings << QString::number(id);
        qDebug() << "[OnnxTextCleaner] tokenized input IDs:" << idStrings.join(", ");
        qDebug() << "[OnnxTextCleaner] tokenized input (decoded):" << QString::fromStdString(tokenizer_->decode(src_ids));
        int64_t S = static_cast<int64_t>(src_ids.size());
        std::vector<int64_t> attn(S, 1);
        Ort::MemoryInfo mem = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);
        std::array<int64_t,2> enc_shape{1, S};
        Ort::Value enc_input_ids = Ort::Value::CreateTensor<int64_t>(mem, src_ids.data(), src_ids.size(), enc_shape.data(), enc_shape.size());
        Ort::Value enc_attn = Ort::Value::CreateTensor<int64_t>(mem, attn.data(), attn.size(), enc_shape.data(), enc_shape.size());
        std::vector<const char*> enc_input_names = {"input_ids", "attention_mask"};
        std::vector<const char*> enc_output_names = {"last_hidden_state"};
        std::vector<Ort::Value> enc_inputs;
        enc_inputs.emplace_back(std::move(enc_input_ids));
        enc_inputs.emplace_back(std::move(enc_attn));
        Ort::RunOptions run_opts{};
        auto enc_outputs = enc_->Run(run_opts, enc_input_names.data(), enc_inputs.data(), enc_inputs.size(), enc_output_names.data(), enc_output_names.size());
        if (enc_outputs.size() != 1 || !enc_outputs[0].IsTensor()) return input;
        Ort::Value enc_hidden = std::move(enc_outputs[0]);
        Ort::Value enc_attn_mask = Ort::Value::CreateTensor<int64_t>(mem, attn.data(), attn.size(), enc_shape.data(), enc_shape.size());

        // 2) Decoder init
        std::vector<int64_t> dec_ids = { tokenizer_->pad_id() };
        std::array<int64_t,2> dec_shape{1, (int64_t)dec_ids.size()};
        Ort::Value dec_input_ids = Ort::Value::CreateTensor<int64_t>(mem, dec_ids.data(), dec_ids.size(), dec_shape.data(), dec_shape.size());

        std::vector<const char*> dec_input_names = {"input_ids", "encoder_hidden_states", "encoder_attention_mask"};
        std::vector<const char*> dec_output_names = {"logits"};

        std::vector<Ort::Value> dec_inputs;
        dec_inputs.emplace_back(std::move(dec_input_ids));
        dec_inputs.emplace_back(std::move(enc_hidden));
        dec_inputs.emplace_back(std::move(enc_attn_mask));

        auto dec_outs = dec_init_->Run(run_opts, dec_input_names.data(), dec_inputs.data(), dec_inputs.size(), dec_output_names.data(), dec_output_names.size());
        if (dec_outs.size() != 1 || !dec_outs[0].IsTensor()) return input;

        std::vector<int64_t> generated;
        generated.reserve(max_len);

        double conf_sum = 0.0; int conf_count = 0;

        for (int step = 0; step < max_len; ++step) {
            Ort::Value& logits = dec_outs[0];
            auto info = logits.GetTensorTypeAndShapeInfo();
            auto shape = info.GetShape();
            if (shape.size() != 3) break;
            int64_t T = shape[1];
            int64_t V = shape[2];
            const float* logptr = logits.GetTensorData<float>();
            const float* last = logptr + (T - 1) * V;

            int64_t best_id = 0;
            float best = last[0];
            for (int64_t i = 1; i < V; ++i) {
                if (last[i] > best) { best = last[i]; best_id = i; }
            }
            if (debug_) {
                std::vector<int64_t> idx(V);
                std::iota(idx.begin(), idx.end(), 0);
                std::partial_sort(idx.begin(), idx.begin()+std::min<int>(debug_topk_, (int)V), idx.end(),
                    [&](int64_t a, int64_t b){ return last[a] > last[b]; });
                qDebug() << "step" << step << "best_id" << (int)best_id << "topk:";
                for (int k = 0; k < std::min<int>(debug_topk_, (int)V); ++k) {
                    qDebug() << "  id" << (int)idx[k] << "logit" << last[idx[k]];
                }
            }
            double denom = 0.0;
            for (int64_t i = 0; i < V; ++i) denom += std::exp((double)last[i] - (double)best);
            double conf = 1.0 / denom;
            conf_sum += conf; conf_count++;

            if (best_id == tokenizer_->eos_id()) break;
            generated.push_back(best_id);
            dec_ids.push_back(best_id);
            dec_shape[1] = (int64_t)dec_ids.size();
            Ort::Value new_dec_input_ids = Ort::Value::CreateTensor<int64_t>(mem, dec_ids.data(), dec_ids.size(), dec_shape.data(), dec_shape.size());
            dec_inputs[0] = std::move(new_dec_input_ids);

            dec_outs = dec_->Run(run_opts, dec_input_names.data(), dec_inputs.data(), dec_inputs.size(), dec_output_names.data(), dec_output_names.size());
            if (dec_outs.size() != 1 || !dec_outs[0].IsTensor()) break;
        }

        last_mean_conf_ = conf_count ? (conf_sum / conf_count) : 0.0;
        if (debug_) qDebug() << "mean confidence:" << last_mean_conf_ << ", tokens:" << (int)generated.size();

        std::string output = tokenizer_->decode(generated);
        qDebug() << "[OnnxTextCleaner] output:" << QString::fromStdString(output);
        return output;
    } catch (const std::exception& e) {
        if (debug_) qDebug() << "OnnxTextCleaner.clean exception:" << e.what();
        return input;
    } catch (...) {
        if (debug_) qDebug() << "OnnxTextCleaner.clean unknown exception";
        return input;
    }
}
