#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/application/AppStateManager.h"
#include "core/models/AppState.h"
#include "core/models/Actor.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"

#include "mocks/MockActorRepository.h"
#include "mocks/MockPropertyRepository.h"
#include "mocks/MockContractRepository.h"
#include "mocks/MockStatementRepository.h"
#include "mocks/MockTransactionRepository.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;

TEST(AppStateManagerInteraction, Save_calls_all_repo_upserts) {
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<NiceMock<MockActorRepository>>();
    auto propRepo = std::make_shared<NiceMock<MockPropertyRepository>>();
    auto contractRepo = std::make_shared<NiceMock<MockContractRepository>>();
    auto stmtRepo = std::make_shared<NiceMock<MockStatementRepository>>();
    auto txRepo = std::make_shared<NiceMock<MockTransactionRepository>>();

    repos.actors = actorRepo;
    repos.properties = propRepo;
    repos.contracts = contractRepo;
    repos.statements = stmtRepo;
    repos.transactions = txRepo;

    AppStateManager mgr(std::move(repos));

    AppState state;
    // create one actor, property, contract, statement, transaction
    auto a = std::make_shared<Actor>(); a->id = ""; a->name = "A";
    auto p = std::make_shared<Property>(); p->id = ""; p->name = "P";
    auto c = std::make_shared<Contract>(); c->id = ""; c->name = "C";
    auto s = std::make_shared<Statement>(); s->id = ""; s->name = "S";
    auto t = std::make_shared<Transaction>(); t->id = ""; t->name = "T";

    state.actors.push_back(a);
    state.properties.push_back(p);
    state.contracts.push_back(c);
    state.statements.push_back(s);
    state.transactions.push_back(t);

    EXPECT_CALL(*actorRepo, upsertActor(_)).Times(1);
    EXPECT_CALL(*propRepo, upsertProperty(_)).Times(1);
    EXPECT_CALL(*contractRepo, upsertContract(_)).Times(1);
    EXPECT_CALL(*stmtRepo, upsertStatement(_)).Times(1);
    EXPECT_CALL(*txRepo, upsertTransaction(_)).Times(1);

    mgr.save(state);
}
