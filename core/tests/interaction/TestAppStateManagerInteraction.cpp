#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/application/AppStateManager.h"
#include "core/models/AppState.h"
#include "core/models/Actor.h"
#include "core/models/Analysis.h"
#include "core/models/Annual.h"
#include "core/models/Property.h"
#include "core/models/Contract.h"
#include "core/models/Statement.h"
#include "core/models/StatementDraft.h"
#include "core/models/Transaction.h"
#include "core/models/TransactionDraft.h"
#include "core/storage/RepositoryBundle.h"

#include "mocks/MockActorRepository.h"
#include "mocks/MockPropertyRepository.h"
#include "mocks/MockContractRepository.h"
#include "mocks/MockStatementRepository.h"
#include "mocks/MockTransactionRepository.h"
#include "mocks/MockAnalysisRepository.h"
#include "mocks/MockAnnualRepository.h"
#include "mocks/MockStatementDraftRepository.h"
#include "mocks/MockTransactionDraftRepository.h"

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::_;
using core::application::AppStateManager;
using core::domain::Actor;
using core::domain::AppState;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::Contract;
using core::domain::Property;
using core::domain::Statement;
using core::domain::StatementDraft;
using core::domain::Transaction;
using core::domain::TransactionDraft;

TEST(AppStateManagerInteraction, SaveCallsAllRepositoryUpserts)
{
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<NiceMock<MockActorRepository>>();
    auto propRepo = std::make_shared<NiceMock<MockPropertyRepository>>();
    auto contractRepo = std::make_shared<NiceMock<MockContractRepository>>();
    auto stmtRepo = std::make_shared<NiceMock<MockStatementRepository>>();
    auto txRepo = std::make_shared<NiceMock<MockTransactionRepository>>();
    auto analysisRepo = std::make_shared<NiceMock<MockAnalysisRepository>>();
    auto annualRepo = std::make_shared<NiceMock<MockAnnualRepository>>();
    auto statementDraftRepo = std::make_shared<NiceMock<MockStatementDraftRepository>>();
    auto transactionDraftRepo = std::make_shared<NiceMock<MockTransactionDraftRepository>>();

    repos.actors = actorRepo;
    repos.properties = propRepo;
    repos.contracts = contractRepo;
    repos.statements = stmtRepo;
    repos.transactions = txRepo;
    repos.analyses = analysisRepo;
    repos.annuals = annualRepo;
    repos.statementDrafts = statementDraftRepo;
    repos.transactionDrafts = transactionDraftRepo;

    AppStateManager mgr(std::move(repos));

    AppState state;
    // create one actor, property, contract, statement, transaction
    auto a = std::make_shared<Actor>(); a->id = ""; a->name = "A";
    auto p = std::make_shared<Property>(); p->id = ""; p->name = "P";
    auto c = std::make_shared<Contract>(); c->id = ""; c->name = "C";
    auto s = std::make_shared<Statement>(); s->id = ""; s->name = "S";
    auto t = std::make_shared<Transaction>(); t->id = ""; t->name = "T";
    auto an = std::make_shared<Analysis>(); an->id = ""; an->name = "A";
    auto yr = std::make_shared<Annual>(); yr->id = ""; yr->year = 2025;
    auto sd = std::make_shared<StatementDraft>(); sd->name = "Draft";
    auto td = std::make_shared<TransactionDraft>(); td->name = "Draft Tx";

    state.actors.push_back(a);
    state.properties.push_back(p);
    state.contracts.push_back(c);
    state.statements.push_back(s);
    state.transactions.push_back(t);
    state.analyses.push_back(an);
    state.annuals.push_back(yr);
    state.statementDrafts.push_back(sd);
    state.transactionDrafts.push_back(td);

    EXPECT_CALL(*actorRepo, upsertActor(_)).Times(1);
    EXPECT_CALL(*propRepo, upsertProperty(_)).Times(1);
    EXPECT_CALL(*contractRepo, upsertContract(_)).Times(1);
    EXPECT_CALL(*stmtRepo, upsertStatement(_)).Times(1);
    EXPECT_CALL(*txRepo, upsertTransaction(_)).Times(1);
    EXPECT_CALL(*analysisRepo, upsertAnalysis(_)).Times(1);
    EXPECT_CALL(*annualRepo, upsertAnnual(_)).Times(1);
    EXPECT_CALL(*statementDraftRepo, clearStatementDrafts()).Times(1);
    EXPECT_CALL(*statementDraftRepo, upsertStatementDraft(_)).Times(1);
    EXPECT_CALL(*transactionDraftRepo, clearTransactionDrafts()).Times(1);
    EXPECT_CALL(*transactionDraftRepo, upsertTransactionDraft(_)).Times(1);

    mgr.save(state);
}

TEST(AppStateManagerInteraction, LoadCallsAllRepositoryGetters)
{
    AppStateManager::Repositories repos;
    auto actorRepo = std::make_shared<NiceMock<MockActorRepository>>();
    auto propRepo = std::make_shared<NiceMock<MockPropertyRepository>>();
    auto contractRepo = std::make_shared<NiceMock<MockContractRepository>>();
    auto stmtRepo = std::make_shared<NiceMock<MockStatementRepository>>();
    auto txRepo = std::make_shared<NiceMock<MockTransactionRepository>>();
    auto analysisRepo = std::make_shared<NiceMock<MockAnalysisRepository>>();
    auto annualRepo = std::make_shared<NiceMock<MockAnnualRepository>>();
    auto statementDraftRepo = std::make_shared<NiceMock<MockStatementDraftRepository>>();
    auto transactionDraftRepo = std::make_shared<NiceMock<MockTransactionDraftRepository>>();

    repos.actors = actorRepo;
    repos.properties = propRepo;
    repos.contracts = contractRepo;
    repos.statements = stmtRepo;
    repos.transactions = txRepo;
    repos.analyses = analysisRepo;
    repos.annuals = annualRepo;
    repos.statementDrafts = statementDraftRepo;
    repos.transactionDrafts = transactionDraftRepo;

    EXPECT_CALL(*actorRepo, getActors()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Actor>>{}));
    EXPECT_CALL(*propRepo, getProperties()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Property>>{}));
    EXPECT_CALL(*contractRepo, getContracts()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Contract>>{}));
    EXPECT_CALL(*stmtRepo, getStatements()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Statement>>{}));
    EXPECT_CALL(*txRepo, getTransactions()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Transaction>>{}));
    EXPECT_CALL(*analysisRepo, getAnalyses()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Analysis>>{}));
    EXPECT_CALL(*annualRepo, getAnnuals()).Times(1).WillOnce(Return(std::vector<std::shared_ptr<Annual>>{}));
    EXPECT_CALL(*statementDraftRepo, getStatementDrafts())
        .Times(1)
        .WillOnce(Return(std::vector<std::shared_ptr<StatementDraft>>{}));
    EXPECT_CALL(*transactionDraftRepo, getTransactionDrafts())
        .Times(1)
        .WillOnce(Return(std::vector<std::shared_ptr<TransactionDraft>>{}));

    AppStateManager mgr(std::move(repos));
    const auto state = mgr.load();

    EXPECT_TRUE(state.empty());
}

TEST(AppStateManagerInteraction, SaveClearsDraftRepositoriesWhenDraftCollectionsAreEmpty)
{
    AppStateManager::Repositories repos;
    auto statementDraftRepo = std::make_shared<NiceMock<MockStatementDraftRepository>>();
    auto transactionDraftRepo = std::make_shared<NiceMock<MockTransactionDraftRepository>>();

    repos.statementDrafts = statementDraftRepo;
    repos.transactionDrafts = transactionDraftRepo;

    EXPECT_CALL(*statementDraftRepo, clearStatementDrafts()).Times(1);
    EXPECT_CALL(*transactionDraftRepo, clearTransactionDrafts()).Times(1);

    AppStateManager mgr(std::move(repos));
    mgr.save(AppState{});
}
