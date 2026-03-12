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

#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/AnalysisResult.h"
#include "core/models/Annual.h"
#include "core/models/AppState.h"
#include "core/models/Contract.h"
#include "core/models/Property.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;
