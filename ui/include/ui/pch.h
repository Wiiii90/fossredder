/**
 * @file ui/include/ui/pch.h
 * @brief Precompiled header for the UI project.
 */

#pragma once

#define NOMINMAX

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sstream>
#include <cctype>
#include <cmath>
#include <set>
#include <map>
#include <iomanip>

#include <QApplication>
#include <QMainWindow>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Contract.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::AnalysisResult;
using core::domain::AnalysisTransaction;
using core::domain::Annual;
using core::domain::catalog::WorkspaceCatalog;
using core::domain::Contract;
using core::domain::DeletionImpact;
using core::application::importing::draft::StatementDraft;
using core::application::importing::draft::TransactionDraft;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;
