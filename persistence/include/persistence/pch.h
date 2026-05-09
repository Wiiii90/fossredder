#pragma once

#define NOMINMAX

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <optional>
#include <map>
#include <sstream>
#include <fstream>
#include <chrono>
#include <mutex>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/application/analysis/RunAnalysisResult.h"
#include "core/domain/entities/Annual.h"
#include "core/application/workspace/AppState.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;
