#pragma once

#include <string>

namespace core::domain {
class Analysis;
}

namespace core::analysis {

std::string resolveAnalysisOutputType(const core::domain::Analysis& analysis);

}
