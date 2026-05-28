/**
 * @file ui/tests/unit/TestBookingState.cpp
 * @brief Tests for the UI BookingState boundary.
 */

#include <gtest/gtest.h>

#include <memory>

#include "core/application/workspace/WorkspaceFacade.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/state/session/BookingState.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

struct BookingStateHarness {
    tests::support::FakeStorageManager* storage = nullptr;
    std::unique_ptr<core::application::WorkspaceFacade> coreFacade;
    std::unique_ptr<WorkspaceFacade> facade;
};

BookingStateHarness makeHarness()
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    auto facade = std::make_unique<WorkspaceFacade>(coreFacade.get());
    return {storagePtr, std::move(coreFacade), std::move(facade)};
}

BookingStateHarness makeHarnessWithCatalog()
{
    auto harness = makeHarness();
    harness.storage->loadedState_.catalog = tests::support::makeWorkspaceCatalog();
    harness.facade->openFile(QStringLiteral("booking-state-loaded.fr"));
    return harness;
}

} // namespace

TEST(BookingStateTest, BKG_ST_001_CreatesStatementWithCurrentTransactionDraft)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("March Statement"));
    state->setTransactionName(QStringLiteral("Rent"));
    state->setTransactionBookingDate(QStringLiteral("2026-03-05"));
    state->setTransactionValuta(QStringLiteral("EUR"));
    state->setTransactionAmountText(QStringLiteral("12.75"));
    state->setTransactionStatusIndex(2);
    state->setTransactionAllocatable(true);

    const QString statementId = state->submit();

    ASSERT_FALSE(statementId.isEmpty());
    EXPECT_EQ(harness.facade->selectedStatementId(), statementId);
    ASSERT_EQ(harness.facade->statementRows().size(), 1);
    ASSERT_EQ(harness.facade->transactionRows().size(), 1);

    const QVariantMap transaction = harness.facade->transaction(harness.facade->selectedTransactionId());
    EXPECT_EQ(transaction.value(QStringLiteral("name")).toString(), QStringLiteral("Rent"));
    EXPECT_DOUBLE_EQ(transaction.value(QStringLiteral("amount")).toDouble(), 12.75);
    EXPECT_EQ(transaction.value(QStringLiteral("status")).toInt(), 2);
    EXPECT_TRUE(transaction.value(QStringLiteral("allocatable")).toBool());
}

TEST(BookingStateTest, BKG_ST_002_CreatesStatementWithoutEmptyDefaultTransaction)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("Empty Statement"));

    const QString statementId = state->submit();

    ASSERT_FALSE(statementId.isEmpty());
    EXPECT_EQ(harness.facade->statementRows().size(), 1);
    EXPECT_TRUE(harness.facade->statementTransactionRows(statementId).isEmpty());
    EXPECT_TRUE(harness.facade->transactionRows().isEmpty());
}

TEST(BookingStateTest, BKG_ST_005_KeepsDecimalAmountTextWhileEditingDraft)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setTransactionAmountText(QStringLiteral("123.40"));
    EXPECT_EQ(state->transactionAmountText(), QStringLiteral("123.40"));

    state->setTransactionAmountText(QStringLiteral("123.41"));
    EXPECT_EQ(state->transactionAmountText(), QStringLiteral("123.41"));
}

TEST(BookingStateTest, BKG_ST_003_BlocksCreateWhenTransactionDraftWouldBeDropped)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("March Statement"));
    state->setTransactionName(QStringLiteral("Rent without date"));

    EXPECT_FALSE(state->canCreate());
    EXPECT_TRUE(state->submit().isEmpty());
    EXPECT_TRUE(harness.facade->statementRows().isEmpty());
    EXPECT_TRUE(harness.facade->transactionRows().isEmpty());
}

TEST(BookingStateTest, BKG_ST_004_CreatesStatementWithMultipleTransactionDrafts)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("March Statement"));
    state->setTransactionName(QStringLiteral("Rent"));
    state->setTransactionBookingDate(QStringLiteral("2026-03-05"));
    state->setTransactionAmountText(QStringLiteral("12.75"));

    state->addTransactionAfterCurrent();
    state->setTransactionName(QStringLiteral("Utilities"));
    state->setTransactionBookingDate(QStringLiteral("2026-03-06"));
    state->setTransactionAmountText(QStringLiteral("34.50"));

    const QString statementId = state->submit();

    ASSERT_FALSE(statementId.isEmpty());
    const QVariantList transactionRows =
        harness.facade->statementTransactionRows(statementId);
    ASSERT_EQ(transactionRows.size(), 2);
    EXPECT_EQ(transactionRows.at(0).toMap().value(QStringLiteral("name")).toString(),
              QStringLiteral("Rent"));
    EXPECT_EQ(transactionRows.at(1).toMap().value(QStringLiteral("name")).toString(),
              QStringLiteral("Utilities"));
    EXPECT_EQ(harness.facade->transactionRows().size(), 2);
}

TEST(BookingStateTest, BKG_ST_006_CommitsDecimalAmountTextWithoutDroppingSeparator)
{
    auto harness = makeHarness();
    harness.facade->newFile(QStringLiteral("booking-state.fr"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("March Statement"));
    state->setTransactionBookingDate(QStringLiteral("2026-03-05"));
    state->setTransactionAmountText(QStringLiteral("123.41"));

    const QString statementId = state->submit();

    ASSERT_FALSE(statementId.isEmpty());
    ASSERT_EQ(harness.facade->transactionRows().size(), 1);
    const QVariantMap transaction =
        harness.facade->transaction(harness.facade->selectedTransactionId());
    EXPECT_DOUBLE_EQ(transaction.value(QStringLiteral("amount")).toDouble(),
                     123.41);
}

TEST(BookingStateTest, BKG_ST_007_ContractSelectionAppliesRelatedActorAndProperties)
{
    auto harness = makeHarnessWithCatalog();
    harness.facade->selection()->setSelectedStatementId(QString());
    harness.facade->selection()->setSelectedTransactionId(QString());
    auto* state = harness.facade->bookingState();

    state->selectContractIndex(1);

    const QVariantMap draft = state->transactionData();
    EXPECT_EQ(draft.value(QStringLiteral("contractId")).toString(), QStringLiteral("contract-1"));
    EXPECT_EQ(draft.value(QStringLiteral("actorId")).toString(), QStringLiteral("actor-1"));
    EXPECT_EQ(draft.value(QStringLiteral("propertyIds")).toStringList(),
              QStringList({QStringLiteral("property-1")}));
}

TEST(BookingStateTest, BKG_ST_008_IncompatibleActorSelectionClearsContract)
{
    auto harness = makeHarnessWithCatalog();
    harness.facade->addActor(QStringLiteral("Other Actor"));
    harness.facade->selection()->setSelectedStatementId(QString());
    harness.facade->selection()->setSelectedTransactionId(QString());
    auto* state = harness.facade->bookingState();

    state->selectContractIndex(1);
    state->selectActorIndex(2);

    const QVariantMap draft = state->transactionData();
    EXPECT_EQ(draft.value(QStringLiteral("actorId")).toString().isEmpty(), false);
    EXPECT_EQ(draft.value(QStringLiteral("contractId")).toString(), QString());
}

TEST(BookingStateTest, BKG_ST_009_IncompatiblePropertySelectionClearsContract)
{
    auto harness = makeHarnessWithCatalog();
    const QString otherPropertyId = harness.facade->addProperty(QStringLiteral("Other Property"));
    harness.facade->selection()->setSelectedStatementId(QString());
    harness.facade->selection()->setSelectedTransactionId(QString());
    auto* state = harness.facade->bookingState();

    state->selectContractIndex(1);
    state->setPropertySelected(otherPropertyId, true);

    const QVariantMap draft = state->transactionData();
    const QStringList selectedPropertyIds =
        draft.value(QStringLiteral("propertyIds")).toStringList();
    EXPECT_TRUE(selectedPropertyIds.contains(QStringLiteral("property-1")));
    EXPECT_TRUE(selectedPropertyIds.contains(otherPropertyId));
    EXPECT_EQ(draft.value(QStringLiteral("contractId")).toString(), QString());
}

TEST(BookingStateTest, BKG_ST_010_EditUpdatePersistsStatementAndTransactionChanges)
{
    auto harness = makeHarnessWithCatalog();
    harness.facade->selection()->setSelectedStatementId(QStringLiteral("statement-1"));
    harness.facade->selection()->setSelectedTransactionId(QStringLiteral("tx-1"));
    auto* state = harness.facade->bookingState();

    state->setStatementName(QStringLiteral("January Updated"));
    state->setTransactionName(QStringLiteral("Rent Updated"));
    state->setTransactionAmountText(QStringLiteral("99.5"));
    ASSERT_TRUE(state->canUpdate());

    state->updateCurrent();

    EXPECT_EQ(harness.facade->selectedStatement()->name(), QStringLiteral("January Updated"));
    const QVariantMap transaction = harness.facade->transaction(QStringLiteral("tx-1"));
    EXPECT_EQ(transaction.value(QStringLiteral("name")).toString(), QStringLiteral("Rent Updated"));
    EXPECT_DOUBLE_EQ(transaction.value(QStringLiteral("amount")).toDouble(), 99.5);
    EXPECT_FALSE(state->canUpdate());
}

} // namespace ui
