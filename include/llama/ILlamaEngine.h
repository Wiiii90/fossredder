#pragma once
#include <vector>
#include <memory>
#include "models/Transaction.h"

class ILlamaEngine {
public:
    virtual ~ILlamaEngine() = default;
};