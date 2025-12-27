#include "ui/state/UiDataSession.h"

UiDataSession::UiDataSession(QObject* parent)
    : QObject(parent)
    , actors_(this)
    , properties_(this)
    , contracts_(this)
    , selectedActor_(this)
    , selectedProperty_(this)
    , selectedContract_(this)
{
}

void UiDataSession::loadFromState(const AppState& state)
{
    actors_.setActors(state.actors);
    properties_.setProperties(state.properties);
    contracts_.setContracts(state.contracts);

    refreshSelectedActor();
    refreshSelectedProperty();
    refreshSelectedContract();
}

void UiDataSession::setSelectedActorId(const QString& id)
{
    if (selectedActorId_ == id) return;
    selectedActorId_ = id;
    refreshSelectedActor();
    emit selectedActorIdChanged();
}

void UiDataSession::setSelectedPropertyId(const QString& id)
{
    if (selectedPropertyId_ == id) return;
    selectedPropertyId_ = id;
    refreshSelectedProperty();
    emit selectedPropertyIdChanged();
}

void UiDataSession::setSelectedContractId(const QString& id)
{
    if (selectedContractId_ == id) return;
    selectedContractId_ = id;
    refreshSelectedContract();
    emit selectedContractIdChanged();
}

void UiDataSession::refreshSelectedActor()
{
    if (selectedActorId_.isEmpty()) {
        selectedActor_.clear();
        return;
    }

    for (const auto& a : actors_.actors()) {
        if (a && QString::fromStdString(a->id) == selectedActorId_) {
            selectedActor_.setActor(QString::fromStdString(a->id),
                                    QString::fromStdString(a->name),
                                    QString::fromStdString(a->type),
                                    QString::fromStdString(a->description));
            return;
        }
    }

    selectedActor_.clear();
}

void UiDataSession::refreshSelectedProperty()
{
    if (selectedPropertyId_.isEmpty()) {
        selectedProperty_.clear();
        return;
    }

    for (const auto& p : properties_.properties()) {
        if (p && QString::fromStdString(p->id) == selectedPropertyId_) {
            selectedProperty_.setProperty(QString::fromStdString(p->id),
                                          QString::fromStdString(p->name),
                                          QString::fromStdString(p->address),
                                          QString::fromStdString(p->description));
            return;
        }
    }

    selectedProperty_.clear();
}

void UiDataSession::refreshSelectedContract()
{
    if (selectedContractId_.isEmpty()) {
        selectedContract_.clear();
        return;
    }

    for (const auto& c : contracts_.contracts()) {
        if (c && QString::fromStdString(c->id) == selectedContractId_) {
            QStringList actorIds;
            for (const auto& aid : c->actorIds) actorIds.push_back(QString::fromStdString(aid));

            QStringList propertyIds;
            for (const auto& pid : c->propertyIds) propertyIds.push_back(QString::fromStdString(pid));

            selectedContract_.setContract(QString::fromStdString(c->id),
                                          QString::fromStdString(c->name),
                                          QString::fromStdString(c->type),
                                          QString::fromStdString(c->description),
                                          actorIds,
                                          propertyIds);
            return;
        }
    }

    selectedContract_.clear();
}
