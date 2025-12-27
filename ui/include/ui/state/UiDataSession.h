#pragma once

#include <QObject>
#include <QString>

#include "core/models/AppState.h"
#include "ui/models/ActorList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/ContractList.h"
#include "ui/state/UiEntitySelection.h"

class UiDataSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(ActorList* actors READ actors CONSTANT)
    Q_PROPERTY(PropertyList* properties READ properties CONSTANT)
    Q_PROPERTY(ContractList* contracts READ contracts CONSTANT)

    Q_PROPERTY(QString selectedActorId READ selectedActorId WRITE setSelectedActorId NOTIFY selectedActorIdChanged)
    Q_PROPERTY(QString selectedPropertyId READ selectedPropertyId WRITE setSelectedPropertyId NOTIFY selectedPropertyIdChanged)
    Q_PROPERTY(QString selectedContractId READ selectedContractId WRITE setSelectedContractId NOTIFY selectedContractIdChanged)

    Q_PROPERTY(UiEntitySelection* selectedActor READ selectedActor CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedProperty READ selectedProperty CONSTANT)
    Q_PROPERTY(UiEntitySelection* selectedContract READ selectedContract CONSTANT)

public:
    explicit UiDataSession(QObject* parent = nullptr);

    ActorList* actors() noexcept { return &actors_; }
    PropertyList* properties() noexcept { return &properties_; }
    ContractList* contracts() noexcept { return &contracts_; }

    void loadFromState(const AppState& state);

    QString selectedActorId() const { return selectedActorId_; }
    QString selectedPropertyId() const { return selectedPropertyId_; }
    QString selectedContractId() const { return selectedContractId_; }

    void setSelectedActorId(const QString& id);
    void setSelectedPropertyId(const QString& id);
    void setSelectedContractId(const QString& id);

    UiEntitySelection* selectedActor() noexcept { return &selectedActor_; }
    UiEntitySelection* selectedProperty() noexcept { return &selectedProperty_; }
    UiEntitySelection* selectedContract() noexcept { return &selectedContract_; }

signals:
    void selectedActorIdChanged();
    void selectedPropertyIdChanged();
    void selectedContractIdChanged();

private:
    ActorList actors_;
    PropertyList properties_;
    ContractList contracts_;

    QString selectedActorId_;
    QString selectedPropertyId_;
    QString selectedContractId_;

    UiEntitySelection selectedActor_;
    UiEntitySelection selectedProperty_;
    UiEntitySelection selectedContract_;

    void refreshSelectedActor();
    void refreshSelectedProperty();
    void refreshSelectedContract();
};
