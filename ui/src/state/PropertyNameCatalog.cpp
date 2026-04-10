/**
 * @file ui/src/state/PropertyNameCatalog.cpp
 * @brief Implementation of the UI PropertyNameCatalog component.
 */

#include "ui/state/PropertyNameCatalog.h"

#include "ui/state/PropertyNameProjection.h"

namespace ui {

void PropertyNameCatalog::rebuild(const PropertyList& properties)
{
    namesById_ = buildPropertyNameIndex(properties);
}

void PropertyNameCatalog::remove(const QString& propertyId)
{
    if (propertyId.isEmpty()) return;
    namesById_.remove(propertyId);
}

QString PropertyNameCatalog::name(const QString& propertyId) const
{
    return resolvePropertyName(namesById_, propertyId);
}

}
