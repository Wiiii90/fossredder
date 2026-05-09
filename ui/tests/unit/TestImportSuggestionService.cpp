/**
 * @file ui/tests/unit/TestImportSuggestionService.cpp
 * @brief Unit tests for the import suggestion ranking service.
 */

#include "gtest/gtest.h"

#include <memory>

#include "core/domain/entities/Actor.h"
#include "core/application/workspace/AppState.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/ports/services/IImportMatcherService.h"
#include "ui/import/ImportSuggestionService.h"

using core::domain::Actor;
using core::domain::AppState;
using core::domain::Contract;
using core::domain::Property;

namespace {

std::shared_ptr<Actor> makeActor(const std::string& id, const std::string& name, const std::string& alias = {})
{
    auto actor = std::make_shared<Actor>();
    actor->id = id;
    actor->name = name;
    if (!alias.empty()) actor->aliases.push_back(core::domain::Alias{.value = alias});
    return actor;
}

std::shared_ptr<Property> makeProperty(const std::string& id, const std::string& name, const std::string& alias = {})
{
    auto property = std::make_shared<Property>();
    property->id = id;
    property->name = name;
    if (!alias.empty()) property->aliases.push_back(core::domain::Alias{.value = alias});
    return property;
}

std::shared_ptr<Contract> makeContract(const std::string& id, const std::string& name, const std::string& type)
{
    auto contract = std::make_shared<Contract>();
    contract->id = id;
    contract->name = name;
    contract->type = type;
    return contract;
}

struct MatcherServiceAdapter final : core::ports::services::IImportMatcherService {
    core::ports::services::ImportMatcherPresentation buildImportSuggestions(const core::domain::WorkspaceState& state,
                                                                            const core::domain::TransactionDraft& transaction) const override
    {
        return core::application::importing::buildImportSuggestions(state, transaction);
    }
};

} // namespace

TEST(ImportSuggestionServiceTests, SuggestsActorPropertyAndContractFromTransactionText)
{
    AppState state;
    state.actors.push_back(makeActor("actor-eon", "EON", "E.ON Energie"));
    state.properties.push_back(makeProperty("property-main", "Musterstraße 1", "Musterstr. 1"));
    state.contracts.push_back(makeContract("contract-gas", "Gasvertrag", "gas"));

    core::domain::TransactionDraft tx;
    tx.name = "EON Abschlag";
    tx.metadata = "E.ON Energie GmbH Musterstraße 1";
    tx.description = "Gas";

    const MatcherServiceAdapter matcher;
    const auto coreSuggestions = matcher.buildImportSuggestions(state, tx);
    const auto suggestions = ui::importing::buildImportSuggestions(coreSuggestions);

    ASSERT_FALSE(suggestions.actor.candidates.empty());
    ASSERT_FALSE(suggestions.property.candidates.empty());
    ASSERT_FALSE(suggestions.contract.candidates.empty());

    EXPECT_EQ(suggestions.actor.candidates.front().entityId, QStringLiteral("actor-eon"));
    EXPECT_EQ(suggestions.property.candidates.front().entityId, QStringLiteral("property-main"));
    EXPECT_EQ(suggestions.contract.candidates.front().entityId, QStringLiteral("contract-gas"));
    EXPECT_GT(suggestions.actor.candidates.front().confidence, 0.0);
    EXPECT_GT(suggestions.property.candidates.front().confidence, 0.0);
    EXPECT_GT(suggestions.contract.candidates.front().confidence, 0.0);
}
