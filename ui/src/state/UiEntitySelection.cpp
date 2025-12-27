#include "ui/state/UiEntitySelection.h"

UiEntitySelection::UiEntitySelection(QObject* parent)
    : QObject(parent)
{
}

void UiEntitySelection::clear()
{
    id_.clear();
    name_.clear();
    type_.clear();
    address_.clear();
    description_.clear();
    actorIds_.clear();
    propertyIds_.clear();
    emit changed();
}

void UiEntitySelection::setActor(const QString& id, const QString& name, const QString& type, const QString& description)
{
    id_ = id;
    name_ = name;
    type_ = type;
    address_.clear();
    description_ = description;
    actorIds_.clear();
    propertyIds_.clear();
    emit changed();
}

void UiEntitySelection::setProperty(const QString& id, const QString& name, const QString& address, const QString& description)
{
    id_ = id;
    name_ = name;
    type_.clear();
    address_ = address;
    description_ = description;
    actorIds_.clear();
    propertyIds_.clear();
    emit changed();
}

void UiEntitySelection::setContract(const QString& id, const QString& name, const QString& type, const QString& description)
{
    setContract(id, name, type, description, {}, {});
}

void UiEntitySelection::setContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                   const QStringList& actorIds, const QStringList& propertyIds)
{
    id_ = id;
    name_ = name;
    type_ = type;
    address_.clear();
    description_ = description;
    actorIds_ = actorIds;
    propertyIds_ = propertyIds;
    emit changed();
}
