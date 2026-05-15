/**
 * @file ui/tests/unit/TestSessionMutationState.cpp
 * @brief Tests for the UI SessionMutationState helper routines.
 */

#include <gtest/gtest.h>

#include <QStringList>

#include "ui/state/mutation/SessionMutationState.h"

namespace ui {

TEST(SessionMutationStateTest, NormalizesStringCollectionsConsistently)
{
    const QVariantList values{QStringLiteral(" alpha "), 17, QStringLiteral("beta"), QStringLiteral(""), true};

    const QVariantList normalized = SessionMutationState::normalizeStrings(values);

    ASSERT_EQ(normalized.size(), 5);
    EXPECT_EQ(normalized.at(0).toString(), QStringLiteral(" alpha "));
    EXPECT_EQ(normalized.at(1).toString(), QStringLiteral("17"));
    EXPECT_EQ(normalized.at(2).toString(), QStringLiteral("beta"));
    EXPECT_EQ(normalized.at(3).toString(), QStringLiteral(""));
    EXPECT_EQ(normalized.at(4).toString(), QStringLiteral("true"));
}

TEST(SessionMutationStateTest, TrimsAndDeduplicatesStringEntries)
{
    const QVariantList values{QStringLiteral("alpha"), QStringLiteral("beta")};

    const QVariantList duplicated = SessionMutationState::addUniqueTrimmed(values, QStringLiteral(" beta "));
    const QVariantList appended = SessionMutationState::addUniqueTrimmed(values, QStringLiteral("gamma"));
    const QVariantList ignored = SessionMutationState::addUniqueTrimmed(values, QStringLiteral("   "));

    ASSERT_EQ(duplicated.size(), 2);
    EXPECT_EQ(duplicated.at(0).toString(), QStringLiteral("alpha"));
    EXPECT_EQ(duplicated.at(1).toString(), QStringLiteral("beta"));
    ASSERT_EQ(appended.size(), 3);
    EXPECT_EQ(appended.at(2).toString(), QStringLiteral("gamma"));
    ASSERT_EQ(ignored.size(), 2);
}

TEST(SessionMutationStateTest, InsertsAndRemovesAtStablePositions)
{
    const QVariantList values{QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral("gamma")};

    const QVariantList insertedFront = SessionMutationState::insertAt(values, -3, QStringLiteral("front"));
    const QVariantList insertedMiddle = SessionMutationState::insertAt(values, 1, QStringLiteral("middle"));
    const QVariantList insertedBack = SessionMutationState::insertAt(values, 99, QStringLiteral("back"));
    const QVariantList removed = SessionMutationState::removeAt(values, 1);
    const QVariantList removedInvalid = SessionMutationState::removeAt(values, 99);
    const QVariantList removedString = SessionMutationState::removeString(values, QStringLiteral("beta"));

    ASSERT_EQ(insertedFront.size(), 4);
    EXPECT_EQ(insertedFront.at(0).toString(), QStringLiteral("front"));
    ASSERT_EQ(insertedMiddle.size(), 4);
    EXPECT_EQ(insertedMiddle.at(1).toString(), QStringLiteral("middle"));
    ASSERT_EQ(insertedBack.size(), 4);
    EXPECT_EQ(insertedBack.last().toString(), QStringLiteral("back"));
    ASSERT_EQ(removed.size(), 2);
    EXPECT_EQ(removed.at(1).toString(), QStringLiteral("gamma"));
    ASSERT_EQ(removedInvalid.size(), 3);
    ASSERT_EQ(removedString.size(), 2);
    EXPECT_EQ(removedString.at(0).toString(), QStringLiteral("alpha"));
    EXPECT_EQ(removedString.at(1).toString(), QStringLiteral("gamma"));
}

TEST(SessionMutationStateTest, NormalizesTransactionDraftStateFamilies)
{
    QVariantMap draft;
    draft.insert(QStringLiteral("name"), QStringLiteral(" Rent "));
    draft.insert(QStringLiteral("bookingDate"), 20260105);
    draft.insert(QStringLiteral("amount"), QStringLiteral("1250.5"));
    draft.insert(QStringLiteral("propertyIds"), QVariantList{QStringLiteral(" property-1 "), 42});
    draft.insert(QStringLiteral("allocatable"), 1);

    const QVariantMap normalized = SessionMutationState::normalizeTransactionDraft(draft);
    EXPECT_EQ(normalized.value(QStringLiteral("name")).toString(), QStringLiteral(" Rent "));
    EXPECT_EQ(normalized.value(QStringLiteral("bookingDate")).toString(), QStringLiteral("20260105"));
    EXPECT_DOUBLE_EQ(normalized.value(QStringLiteral("amount")).toDouble(), 1250.5);
    ASSERT_EQ(normalized.value(QStringLiteral("propertyIds")).toList().size(), 2);
    EXPECT_EQ(normalized.value(QStringLiteral("propertyIds")).toList().at(0).toString(), QStringLiteral(" property-1 "));
    EXPECT_TRUE(normalized.value(QStringLiteral("allocatable")).toBool());

    const QVariantMap emptyDraft = SessionMutationState::emptyTransactionDraft();
    EXPECT_TRUE(SessionMutationState::transactionDraftHasContent(normalized));
    EXPECT_FALSE(SessionMutationState::transactionDraftHasContent(emptyDraft));

    const QVariantMap draftsState = SessionMutationState::createDraftListState(
        QVariantList{draft},
        3,
        emptyDraft);
    ASSERT_TRUE(draftsState.contains(QStringLiteral("drafts")));
    ASSERT_TRUE(draftsState.contains(QStringLiteral("index")));
    EXPECT_EQ(draftsState.value(QStringLiteral("index")).toInt(), 0);

    const QVariantMap inserted = SessionMutationState::insertDraftAfterCurrent(
        QVariantList{draft},
        0,
        emptyDraft);
    ASSERT_EQ(inserted.value(QStringLiteral("drafts")).toList().size(), 2);
    EXPECT_EQ(inserted.value(QStringLiteral("index")).toInt(), 1);

    const QVariantMap removed = SessionMutationState::removeDraftAt(
        QVariantList{draft, emptyDraft},
        0,
        emptyDraft);
    ASSERT_EQ(removed.value(QStringLiteral("drafts")).toList().size(), 1);
    EXPECT_EQ(removed.value(QStringLiteral("index")).toInt(), 0);

    const QVariantMap current = SessionMutationState::currentDraftState(
        QVariantList{draft},
        0,
        emptyDraft);
    ASSERT_TRUE(current.contains(QStringLiteral("draft")));
    EXPECT_EQ(current.value(QStringLiteral("draft")).toMap().value(QStringLiteral("name")).toString(),
              QStringLiteral(" Rent "));
}

} // namespace ui

