#pragma once
#include "onnx/OnnxTextCleaner.h"

// Factory to read model paths and tokenizer type from environment variables (.env)
OnnxTextCleaner::ModelPaths make_model_paths_from_env();