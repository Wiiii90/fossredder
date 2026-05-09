#pragma once

#include "core/application/import/draft/DraftMatcher.h"

namespace core::ports::services {

using ImportMatcherPresentation = core::application::importing::draft::DraftImportSuggestions;

class IImportMatcherService {
public:
    virtual ~IImportMatcherService() = default;

    virtual ImportMatcherPresentation buildImportSuggestions(const core::domain::WorkspaceState& state,
                                                             const core::application::importing::draft::TransactionDraft& transaction) const = 0;

    virtual core::application::importing::draft::DraftTextSignals buildDraftTextSignals(const core::domain::WorkspaceState& state,
                                                                                const core::application::importing::draft::TransactionDraft& transaction) const = 0;
    virtual core::application::importing::draft::DraftDerivedState buildDraftDerivedState(const core::domain::WorkspaceState& state,
                                                                                   const core::application::importing::draft::DraftLinkSelection& selection) const = 0;
    virtual std::string resolveActorId(const core::domain::WorkspaceState& state, const std::string& text) const = 0;
    virtual std::string resolveContractId(const core::domain::WorkspaceState& state, const std::string& text) const = 0;
    virtual bool contractIsFullyAllocatable(const core::domain::WorkspaceState& state, const std::string& contractId) const = 0;
    virtual core::domain::WorkspaceState withFallbackState(core::domain::WorkspaceState primary,
                                                           const core::domain::WorkspaceState& fallback) const = 0;
    virtual std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata) const = 0;
};

} // namespace core::ports::services
