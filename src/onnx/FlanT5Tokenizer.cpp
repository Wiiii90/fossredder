#include "onnx/FlanT5Tokenizer.h"
#include <QDebug>
#include "util/Env.h"
#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <regex>
#ifdef _WIN32
#include <windows.h>
#endif

// Hilfsfunktion: UTF-8 zu wstring (nur Windows)
#ifdef _WIN32
static std::wstring debug_utf8_to_wstring(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    if (len <= 0) return {};
    std::wstring w;
    w.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), len);
    return w;
}
#endif

void debug_decoder_input_names_from_env() {
    std::string decoder_path = env::get("FLANT5_DECODER_ONNX", "");
    if (decoder_path.empty()) {
        qDebug() << "[ONNX Decoder] FLANT5_DECODER_ONNX not set in .env";
        return;
    }
    try {
        Ort::Env ort_env(ORT_LOGGING_LEVEL_WARNING, "decoder-input-names");
        Ort::SessionOptions opts;
#ifdef _WIN32
        std::wstring wpath = debug_utf8_to_wstring(decoder_path);
        Ort::Session session(ort_env, wpath.c_str(), opts);
#else
        Ort::Session session(ort_env, decoder_path.c_str(), opts);
#endif
        size_t num_inputs = session.GetInputCount();
        qDebug() << "[ONNX Decoder] Input count:" << (int)num_inputs;
        Ort::AllocatorWithDefaultOptions allocator;
        for (size_t i = 0; i < num_inputs; ++i) {
            Ort::AllocatedStringPtr name = session.GetInputNameAllocated(i, allocator);
            qDebug() << "[ONNX Decoder] Input name:" << name.get();
        }
    } catch (const std::exception& e) {
        qDebug() << "[ONNX Decoder] Exception:" << e.what();
    }
}

FlanT5Tokenizer::FlanT5Tokenizer(const std::string& tokenizer_json_path) {
    debug_decoder_input_names_from_env(); // Debug-Ausgabe beim Tokenizer-Konstruktor
    load_tokenizer(tokenizer_json_path);
}

void FlanT5Tokenizer::load_tokenizer(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("FlanT5Tokenizer: Tokenizer-JSON konnte nicht geöffnet werden: " + path);
    }
    nlohmann::json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("FlanT5Tokenizer: Fehler beim Parsen der Tokenizer-JSON: ") + e.what());
    }
    bool found_pad = false, found_eos = false;
    // Spezial-Tokens aus added_tokens
    if (j.contains("added_tokens") && j["added_tokens"].is_array()) {
        for (const auto& tok : j["added_tokens"]) {
            if (tok.contains("content") && tok.contains("id")) {
                std::string token = tok["content"];
                int64_t id = tok["id"];
                token_to_id_[token] = id;
                id_to_token_[id] = token;
                if (token == "<pad>") { pad_id_ = id; found_pad = true; }
                if (token == "</s>") { eos_id_ = id; found_eos = true; }
            }
        }
    }
    // Haupt-Vokabular: model.vocab oder vocab
    if (j.contains("model") && j["model"].contains("vocab") && j["model"]["vocab"].is_object()) {
        for (auto it = j["model"]["vocab"].begin(); it != j["model"]["vocab"].end(); ++it) {
            token_to_id_[it.key()] = it.value();
            id_to_token_[it.value()] = it.key();
            if (it.key() == "<pad>") { pad_id_ = it.value(); found_pad = true; }
            if (it.key() == "</s>") { eos_id_ = it.value(); found_eos = true; }
        }
    } else if (j.contains("vocab") && j["vocab"].is_object()) {
        for (auto it = j["vocab"].begin(); it != j["vocab"].end(); ++it) {
            token_to_id_[it.key()] = it.value();
            id_to_token_[it.value()] = it.key();
            if (it.key() == "<pad>") { pad_id_ = it.value(); found_pad = true; }
            if (it.key() == "</s>") { eos_id_ = it.value(); found_eos = true; }
        }
    }
    if (!found_pad || !found_eos) throw std::runtime_error("FlanT5Tokenizer: pad/eos Token fehlt im Vokabular!");
    if (token_to_id_.empty()) throw std::runtime_error("FlanT5Tokenizer: Kein Vokabular geladen!");
}

std::vector<int64_t> FlanT5Tokenizer::encode(const std::string& text) const {
    // SentencePiece-artige Vorverarbeitung: Vor jedem Wort ein '▁' (UTF-8) einfügen
    std::string norm_text;
    bool in_word = false;
    const std::string sp_piece = "\xE2\x96\x81"; // UTF-8 für U+2581
    for (char c : text) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            in_word = false;
        } else {
            if (!in_word) norm_text += sp_piece;
            in_word = true;
        }
        norm_text += c;
    }
    // Tokenisierung: längstes passendes Token ab jeder Position
    std::vector<int64_t> ids;
    size_t pos = 0;
    while (pos < norm_text.size()) {
        size_t best_len = 0;
        int64_t best_id = -1;
        std::string best_token;
        for (const auto& kv : token_to_id_) {
            const std::string& token = kv.first;
            if (token.empty()) continue;
            if (norm_text.compare(pos, token.size(), token) == 0 && token.size() > best_len) {
                best_len = token.size();
                best_id = kv.second;
                best_token = token;
            }
        }
        if (best_id != -1) {
            qDebug() << "[Tokenizer] Match at pos" << (int)pos << ": '" << QString::fromStdString(norm_text.substr(pos, best_len)) << "' -> token '" << QString::fromStdString(best_token) << "' (id " << best_id << ")";
            ids.push_back(best_id);
            pos += best_len;
        } else {
            auto unk = token_to_id_.find("<unk>");
            qDebug() << "[Tokenizer] No match at pos" << (int)pos << ": '" << QString::fromStdString(norm_text.substr(pos, 8)) << "' -> <unk> (id " << (unk != token_to_id_.end() ? unk->second : -1) << ")";
            if (unk != token_to_id_.end()) {
                ids.push_back(unk->second);
            }
            pos += 1;
        }
        // Überspringe Whitespaces
        while (pos < norm_text.size() && std::isspace(static_cast<unsigned char>(norm_text[pos]))) ++pos;
    }
    return ids;
}

std::string FlanT5Tokenizer::decode(const std::vector<int64_t>& ids) const {
    std::string out;
    for (auto id : ids) {
        auto it = id_to_token_.find(id);
        if (it != id_to_token_.end()) {
            if (!out.empty()) out += " ";
            out += it->second;
        }
    }
    return out;
}

void debug_tokenizer_and_onnx() {
    // Lade .env
    bool loaded = env::load_dotenv("C:/coding/fossredder/.env", false);
    qDebug() << "[DEBUG] dotenv loaded:" << loaded;
    std::string tokenizer_path = env::get("FLANT5_TOKENIZER_JSON", "tokenizer.json");
    std::string encoder_path = env::get("FLANT5_ENCODER_ONNX", "");

    // Tokenizer test
    try {
        FlanT5Tokenizer tokenizer(tokenizer_path);
        std::string test_text = "Alte Leipziger";
        auto ids = tokenizer.encode(test_text);
        qDebug() << "[Tokenizer] Text:" << QString::fromStdString(test_text);
        std::ostringstream oss;
        for (size_t i = 0; i < ids.size(); ++i) {
            oss << ids[i];
            if (i + 1 < ids.size()) oss << ", ";
        }
        qDebug() << "[Tokenizer] Token IDs:" << QString::fromStdString(oss.str());
        QStringList tokenStrs;
        for (auto id : ids) tokenStrs << QString::fromStdString(tokenizer.token_for_id(id));
        qDebug() << "[Tokenizer] Tokens:" << tokenStrs.join(", ");
    } catch (const std::exception& e) {
        qDebug() << "[Tokenizer] Exception:" << e.what();
    }

    // ONNX model input names logging
    try {
        if (!encoder_path.empty()) {
            Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "input-names");
            Ort::SessionOptions opts;
            auto wpath = debug_utf8_to_wstring(encoder_path);
            Ort::Session session(env, wpath.c_str(), opts);
            size_t num_inputs = session.GetInputCount();
            qDebug() << "[ONNX] Input count:" << (int)num_inputs;
            Ort::AllocatorWithDefaultOptions allocator;
            for (size_t i = 0; i < num_inputs; ++i) {
                Ort::AllocatedStringPtr name = session.GetInputNameAllocated(i, allocator);
                qDebug() << "[ONNX] Input name:" << name.get();
            }
        } else {
            qDebug() << "[ONNX] Encoder path not found in .env";
        }
    } catch (const std::exception& e) {
        qDebug() << "[ONNX] Exception:" << e.what();
    }
}
