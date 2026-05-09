#pragma once

#include "core/application/import/draft/TransactionDraft.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace core::domain { class Statement; }

namespace core::ports::presenters {

struct ImportPresentation {
    std::shared_ptr<core::domain::Statement> data;
    std::vector<core::application::importing::draft::TransactionDraft> transactions;
    std::map<std::string, std::vector<uint8_t>> artifacts;
};

class IImportPresenter {
public:
    virtual ~IImportPresenter() = default;

    virtual ImportPresentation present(const ImportPresentation& result) const = 0;
};

} // namespace core::ports::presenters
