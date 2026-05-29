/**
 * @file ui/tests/unit/TestWorkspaceFacade.cpp
 * @brief Tests for the UI WorkspaceFacade boundary.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "support/FakeStorageManager.h"
#include "support/WorkspaceTestData.h"
#include "ui/adapters/core/WorkspaceRowProjector.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace {

template <typename Entity>
QStringList aliasValues(const std::shared_ptr<Entity>& entity)
{
    QStringList values;
    if (!entity) {
        return values;
    }

    for (const auto& alias : entity->aliases()) {
        values.push_back(QString::fromStdString(alias.value()));
    }
    return values;
}

template <typename Entity>
std::shared_ptr<Entity> findById(const std::vector<std::shared_ptr<Entity>>& items, const std::string& id)
{
    const auto it = std::find_if(items.begin(), items.end(), [&](const auto& item) {
        return item && item->id() == id;
    });
    return it == items.end() ? nullptr : *it;
}

} // namespace

namespace ui {

TEST(WorkspaceFacadeTest, LoadsTheWorkspaceProjectionAndSelectionState)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    const int initialRevision = facade.dataRevision();
    facade.loadFromState(tests::support::makeWorkspaceCatalog());
    EXPECT_GT(facade.dataRevision(), initialRevision);

    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.properties()->rowCount(), 1);
    EXPECT_EQ(facade.contracts()->rowCount(), 1);
    EXPECT_EQ(facade.statements()->rowCount(), 1);
    EXPECT_EQ(facade.transactions()->rowCount(), 2);
    EXPECT_EQ(facade.analyses()->rowCount(), 1);
    EXPECT_EQ(facade.annuals()->rowCount(), 1);

    facade.setSelectedActorId(QStringLiteral("actor-1"));
    facade.setSelectedPropertyId(QStringLiteral("property-1"));
    facade.setSelectedContractId(QStringLiteral("contract-1"));
    facade.setSelectedStatementId(QStringLiteral("statement-1"));
    facade.setSelectedTransactionId(QStringLiteral("tx-1"));
    facade.setSelectedAnalysisId(QStringLiteral("analysis-1"));
    facade.setSelectedAnnualId(QStringLiteral("annual-1"));

    ASSERT_NE(facade.selectedActor(), nullptr);
    ASSERT_NE(facade.selectedTransaction(), nullptr);
    ASSERT_NE(facade.selectedAnalysis(), nullptr);
    EXPECT_EQ(facade.selectedActor()->id(), QStringLiteral("actor-1"));
    EXPECT_EQ(facade.selectedActor()->name(), QStringLiteral("Main Actor"));
    EXPECT_EQ(facade.selectedTransaction()->id(), QStringLiteral("tx-1"));
    EXPECT_EQ(facade.selectedTransaction()->statementId(), QStringLiteral("statement-1"));
    EXPECT_EQ(facade.selectedAnalysis()->id(), QStringLiteral("analysis-1"));

    EXPECT_EQ(facade.statementTransactionIds(QStringLiteral("statement-1")),
              QVariantList({QStringLiteral("tx-1"), QStringLiteral("tx-2")}));
    const QVariantMap txPayload = facade.transaction(QStringLiteral("tx-1"));
    EXPECT_EQ(txPayload.value(QStringLiteral("id")).toString(),
              QStringLiteral("tx-1"));
    EXPECT_EQ(txPayload.value(QStringLiteral("contractId")).toString(),
              QStringLiteral("contract-1"));
    EXPECT_EQ(facade.annual(QStringLiteral("annual-1")).value(QStringLiteral("id")).toString(),
              QStringLiteral("annual-1"));

    const QVariantList rows{
        QVariantMap{{QStringLiteral("id"), QStringLiteral("a")}, {QStringLiteral("display"), QStringLiteral("Alpha")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("b")}, {QStringLiteral("display"), QStringLiteral("Beta")}},
        QVariantMap{{QStringLiteral("id"), QStringLiteral("c")}, {QStringLiteral("display"), QStringLiteral("Gamma")}}
    };
    const QVariantMap orderedSelection = ui::orderedSelectionState(rows,
                                                                   QVariantList{QStringLiteral("c"), QStringLiteral("a")},
                                                                   1,
                                                                   QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("orderIds")).toList(),
              QVariantList({QStringLiteral("c"), QStringLiteral("a"), QStringLiteral("b")}));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("index")).toInt(), 2);
    EXPECT_EQ(orderedSelection.value(QStringLiteral("id")).toString(), QStringLiteral("b"));
    EXPECT_EQ(orderedSelection.value(QStringLiteral("currentId")).toString(), QStringLiteral("b"));

    EXPECT_EQ(storagePtr->currentPath(), std::string());
    facade.newFile(QStringLiteral("workspace-a.fr"));
    EXPECT_EQ(facade.currentPath(), QStringLiteral("workspace-a.fr"));
    EXPECT_EQ(storagePtr->currentPath(), std::string("workspace-a.fr"));
}

TEST(WorkspaceFacadeTest, RoutesMutationsThroughTheCoreBoundaryAndRefreshesUIRows)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    const int initialRevision = facade.dataRevision();
    EXPECT_EQ(facade.actors()->rowCount(), 0);

    const QString createdId = facade.addActor(QStringLiteral("Second Actor"),
                                              QStringList{QStringLiteral("Actor Two")});
    ASSERT_FALSE(createdId.isEmpty());
    EXPECT_GT(facade.dataRevision(), initialRevision);
    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.actorRows().size(), 1);
    EXPECT_EQ(facade.actors()->data(facade.actors()->index(0, 0), ActorList::NameRole).toString(),
              QStringLiteral("Second Actor"));

    const QString thirdId = facade.addActor(QStringLiteral("Third Actor"));
    ASSERT_FALSE(thirdId.isEmpty());
    EXPECT_EQ(facade.actors()->rowCount(), 2);
    const int revisionAfterSecondAdd = facade.dataRevision();

    facade.updateActor(thirdId, QStringLiteral("Third Actor Updated"));
    EXPECT_GT(facade.dataRevision(), revisionAfterSecondAdd);
    EXPECT_EQ(facade.actors()->data(facade.actors()->index(1, 0), ActorList::NameRole).toString(),
              QStringLiteral("Third Actor Updated"));

    const int revisionAfterUpdate = facade.dataRevision();
    facade.deleteActor(createdId);
    EXPECT_GT(facade.dataRevision(), revisionAfterUpdate);
    EXPECT_EQ(facade.actors()->rowCount(), 1);
    EXPECT_EQ(facade.actors()->data(facade.actors()->index(0, 0), ActorList::NameRole).toString(),
              QStringLiteral("Third Actor Updated"));

    const int revisionBeforeContract = facade.dataRevision();
    const QString contractId = facade.addContract(QStringLiteral("Lease Agreement"),
                                                  QStringLiteral("lease"));
    ASSERT_FALSE(contractId.isEmpty());
    EXPECT_GT(facade.dataRevision(), revisionBeforeContract);
    EXPECT_EQ(facade.contracts()->rowCount(), 1);

    const int revisionBeforeContractDelete = facade.dataRevision();
    facade.deleteContract(contractId);
    EXPECT_GT(facade.dataRevision(), revisionBeforeContractDelete);
    EXPECT_EQ(facade.contracts()->rowCount(), 0);
}

TEST(WorkspaceFacadeTest, AnalysisUpdatePreservesOrReplacesCalculationAdjustments)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());
    storagePtr->loadedState_.catalog = tests::support::makeWorkspaceCatalog();
    coreFacade->openFile("workspace-analysis.fr");

    facade.updateAnalysis(QStringLiteral("analysis-1"),
                          QStringLiteral("Monthly Analysis Updated"),
                          QStringLiteral("tabular"),
                          QStringLiteral("{\"groupBy\":\"month\"}"),
                          QStringLiteral("{}"),
                          QStringLiteral("csv"),
                          true,
                          QStringLiteral("{}"),
                          QStringLiteral("[\"tx-1\"]"));

    auto snapshot = coreFacade->workspaceSnapshot();
    ASSERT_EQ(snapshot.analyses.size(), 1U);
    ASSERT_EQ(snapshot.analyses.front().adjustments.size(), 1U);
    EXPECT_EQ(snapshot.analyses.front().adjustments.front().first, std::string("actor-1"));
    EXPECT_DOUBLE_EQ(snapshot.analyses.front().adjustments.front().second, 19.25);

    facade.updateAnalysis(QStringLiteral("analysis-1"),
                          QStringLiteral("Monthly Analysis Updated"),
                          QStringLiteral("tabular"),
                          QStringLiteral("{\"groupBy\":\"month\"}"),
                          QStringLiteral("{}"),
                          QStringLiteral("csv"),
                          false,
                          QStringLiteral("{}"),
                          QStringLiteral("[\"tx-1\"]"),
                          QStringLiteral("{\"tx-1\":1500.0}"));

    snapshot = coreFacade->workspaceSnapshot();
    ASSERT_EQ(snapshot.analyses.size(), 1U);
    ASSERT_EQ(snapshot.analyses.front().adjustments.size(), 1U);
    EXPECT_EQ(snapshot.analyses.front().adjustments.front().first, std::string("tx-1"));
    EXPECT_DOUBLE_EQ(snapshot.analyses.front().adjustments.front().second, 1500.0);
    EXPECT_FALSE(snapshot.analyses.front().includeCalculationAdjustments);
}

TEST(WorkspaceFacadeTest, PersistsActorAliasesAcrossSelectionChanges)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    facade.newFile(QStringLiteral("workspace-actor.fr"));

    const QString actorId = facade.addActor(QStringLiteral("Main Actor"),
                                            QStringList{QStringLiteral("Primary Actor"),
                                                        QStringLiteral("Main Actor")});
    const QString otherActorId = facade.addActor(QStringLiteral("Backup Actor"),
                                                 QStringList{QStringLiteral("Backup Actor")});
    ASSERT_FALSE(actorId.isEmpty());
    ASSERT_FALSE(otherActorId.isEmpty());

    facade.setSelectedActorId(actorId);

    const QStringList updatedAliases{
        QStringLiteral("Primary Actor"),
        QStringLiteral("Main Actor"),
        QStringLiteral("Actor Fresh Alias")
    };

    const QString savedId = facade.saveActor(actorId,
                                             QStringLiteral("Main Actor"),
                                             updatedAliases,
                                             QStringList{});
    EXPECT_EQ(savedId, actorId);

    const auto savedActor = findById(storagePtr->savedState_.catalog.actors(), actorId.toStdString());
    ASSERT_NE(savedActor, nullptr);
    EXPECT_EQ(aliasValues(savedActor), updatedAliases);

    facade.setSelectedActorId(otherActorId);
    ASSERT_NE(facade.selectedActor(), nullptr);
    EXPECT_EQ(facade.selectedActor()->aliases(), QStringList({QStringLiteral("Backup Actor")}));

    facade.setSelectedActorId(actorId);
    ASSERT_NE(facade.selectedActor(), nullptr);
    EXPECT_EQ(facade.selectedActor()->aliases(), updatedAliases);
}

TEST(WorkspaceFacadeTest, PersistsPropertyAliasesAcrossSelectionChanges)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    facade.newFile(QStringLiteral("workspace-property.fr"));

    const QString propertyId = facade.addProperty(QStringLiteral("Primary Property"),
                                                  QStringList{QStringLiteral("Property Alias")});
    const QString otherPropertyId = facade.addProperty(QStringLiteral("Backup Property"),
                                                       QStringList{QStringLiteral("Backup Property")});
    ASSERT_FALSE(propertyId.isEmpty());
    ASSERT_FALSE(otherPropertyId.isEmpty());

    facade.setSelectedPropertyId(propertyId);

    const QStringList updatedAliases{
        QStringLiteral("Property Alias"),
        QStringLiteral("Property Fresh Alias")
    };

    const QString savedId = facade.saveProperty(propertyId,
                                                QStringLiteral("Primary Property"),
                                                updatedAliases,
                                                QStringList{});
    EXPECT_EQ(savedId, propertyId);

    const auto savedProperty = findById(storagePtr->savedState_.catalog.properties(), propertyId.toStdString());
    ASSERT_NE(savedProperty, nullptr);
    EXPECT_EQ(aliasValues(savedProperty), updatedAliases);

    facade.setSelectedPropertyId(otherPropertyId);
    ASSERT_NE(facade.selectedProperty(), nullptr);
    EXPECT_EQ(facade.selectedProperty()->aliases(), QStringList({QStringLiteral("Backup Property")}));

    facade.setSelectedPropertyId(propertyId);
    ASSERT_NE(facade.selectedProperty(), nullptr);
    EXPECT_EQ(facade.selectedProperty()->aliases(), updatedAliases);
}

TEST(WorkspaceFacadeTest, PersistsContractAliasesAcrossSelectionChanges)
{
    auto storage = std::make_unique<tests::support::FakeStorageManager>();
    auto* storagePtr = storage.get();
    auto coreFacade = std::make_unique<core::application::WorkspaceFacade>(std::move(storage));
    WorkspaceFacade facade(coreFacade.get());

    facade.newFile(QStringLiteral("workspace-contract.fr"));

    const QString contractId = facade.addContract(QStringLiteral("Lease Contract"),
                                                  QStringLiteral("lease"),
                                                  QStringList{},
                                                  QStringList{},
                                                  QStringList{QStringLiteral("Lease")});
    const QString otherContractId = facade.addContract(QStringLiteral("Backup Contract"),
                                                       QStringLiteral("lease"),
                                                       QStringList{},
                                                       QStringList{},
                                                       QStringList{QStringLiteral("Backup Contract")});
    ASSERT_FALSE(contractId.isEmpty());
    ASSERT_FALSE(otherContractId.isEmpty());

    facade.setSelectedContractId(contractId);

    const QStringList updatedAliases{
        QStringLiteral("Lease"),
        QStringLiteral("Lease Fresh Alias")
    };

    const QString savedId = facade.saveContract(contractId,
                                                QStringLiteral("Lease Contract"),
                                                QStringLiteral("lease"),
                                                QStringList{},
                                                QStringList{},
                                                updatedAliases);
    EXPECT_EQ(savedId, contractId);

    const auto savedContract = findById(storagePtr->savedState_.catalog.contracts(), contractId.toStdString());
    ASSERT_NE(savedContract, nullptr);
    EXPECT_EQ(aliasValues(savedContract), updatedAliases);

    facade.setSelectedContractId(otherContractId);
    ASSERT_NE(facade.selectedContract(), nullptr);
    EXPECT_EQ(facade.selectedContract()->aliases(), QStringList({QStringLiteral("Backup Contract")}));

    facade.setSelectedContractId(contractId);
    ASSERT_NE(facade.selectedContract(), nullptr);
    EXPECT_EQ(facade.selectedContract()->aliases(), updatedAliases);
}

} // namespace ui
