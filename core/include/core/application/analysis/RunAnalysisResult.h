#pragma once

#include "core/ports/presenters/IAnalysisPresenter.h"

namespace core::application::analysis {
using AnalysisTransaction = core::ports::presenters::AnalysisTransactionPresentation;
using RunAnalysisResult = core::ports::presenters::AnalysisPresentation;
} // namespace core::application::analysis

namespace core::domain {
using AnalysisTransaction = core::application::analysis::AnalysisTransaction;
using AnalysisResult = core::application::analysis::RunAnalysisResult;
}
