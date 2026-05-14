/**
 * @file core/include/core/application/import/draft/IImportMatcherService.h
 * @brief Declares the import-draft matcher interface used by the import workflow and UI draft editing.
 */

#pragma once

#include "core/application/import/draft/DraftMatcher.h"

namespace core::application::importing::draft {

using ImportMatcherPresentation = DraftImportSuggestions;

/**
 * @brief Matches imported transactions against the current workspace state.
 */
class IImportMatcherService {
public:
    virtual ~IImportMatcherService() = default;

    virtual ImportMatcherPresentation buildImportSuggestions(const core::domain::catalog::WorkspaceCatalog& state,
                                                             const TransactionDraft& transaction) const = 0;

    virtual DraftTextSignals buildDraftTextSignals(const core::domain::catalog::WorkspaceCatalog& state,
                                                   const TransactionDraft& transaction) const = 0;
    virtual DraftDerivedState buildDraftDerivedState(const core::domain::catalog::WorkspaceCatalog& state,
                                                     const DraftLinkSelection& selection) const = 0;
    virtual std::string resolveActorId(const core::domain::catalog::WorkspaceCatalog& state, const std::string& text) const = 0;
    virtual std::string resolveContractId(const core::domain::catalog::WorkspaceCatalog& state, const std::string& text) const = 0;
    virtual bool contractIsFullyAllocatable(const core::domain::catalog::WorkspaceCatalog& state, const std::string& contractId) const = 0;
    virtual core::domain::catalog::WorkspaceCatalog withFallbackState(core::domain::catalog::WorkspaceCatalog primary,
                                                                       const core::domain::catalog::WorkspaceCatalog& fallback) const = 0;
    virtual std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata) const = 0;
};

}
