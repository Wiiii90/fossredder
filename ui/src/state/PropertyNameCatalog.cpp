#include "ui/state/PropertyNameCatalog.h"

#include "ui/models/PropertyList.h"

namespace ui {

void PropertyNameCatalog::rebuild(const PropertyList& properties)
{
    namesById_.clear();
    const auto& rows = properties.properties();
    namesById_.reserve(static_cast<int>(rows.size()));
    for (const auto& property : rows) {
        if (!property) continue;
        namesById_.insert(QString::fromStdString(property->id), QString::fromStdString(property->name));
    }
}

void PropertyNameCatalog::remove(const QString& propertyId)
{
    if (propertyId.isEmpty()) return;
    namesById_.remove(propertyId);
}

QString PropertyNameCatalog::name(const QString& propertyId) const
{
    if (propertyId.isEmpty()) return {};
    const auto it = namesById_.find(propertyId);
    return it == namesById_.end() ? propertyId : it.value();
}

}
