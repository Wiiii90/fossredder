#include "utils/Env.h"
#include "onnx/OnnxTextCleaner.h"
#include "models/Config.h"

OnnxTextCleaner::ModelPaths make_model_paths_from_env() {
    OnnxTextCleaner::ModelPaths p{};
    std::string type = env::get("ONNX_TOKENIZER_TYPE", "flant5");
    p.tokenizer_type = type;
    if (type == "byt5") {
        p.encoder = env::get("BYT5_ENCODER_ONNX", "");
        p.init_decoder = env::get("BYT5_DECODER_INIT_ONNX", "");
        p.decoder = env::get("BYT5_DECODER_ONNX", "");
    } else if (type == "mt5") {
        p.encoder = env::get("MT5_ENCODER_ONNX", "");
        p.init_decoder = env::get("MT5_DECODER_INIT_ONNX", "");
        p.decoder = env::get("MT5_DECODER_ONNX", "");
        p.tokenizer_json = env::get("MT5_TOKENIZER_JSON", "");
        p.sp_model = env::get("MT5_SENTENCEPIECE", "");
    } else { // flant5 default
        p.encoder = env::get("FLANT5_ENCODER_ONNX", "");
        p.init_decoder = env::get("FLANT5_DECODER_INIT_ONNX", "");
        p.decoder = env::get("FLANT5_DECODER_ONNX", "");
        p.tokenizer_json = env::get("FLANT5_TOKENIZER_JSON", "");
    }
    return p;
}
