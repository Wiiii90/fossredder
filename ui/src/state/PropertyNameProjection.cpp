/**
 * @file ui/src/state/PropertyNameProjection.cpp
 * @brief Implements helpers for property-name cache projection and lookup.
 */

#include "ui/state/PropertyNameProjection.h"

#include "ui/models/PropertyList.h"

namespace ui {

QHash<QString, QString> buildPropertyNameIndex(const PropertyList& properties)
{
    QHash<QString, QString> namesById;
    const auto& rows = properties.properties();
    namesById.reserve(static_cast<int>(rows.size()));
    for (const auto& property : rows) {
        if (!property) continue;
        namesById.insert(QString::fromStdString(property->id), QString::fromStdString(property->name));
    }
    return namesById;
}

QString resolvePropertyName(const QHash<QString, QString>& namesById, const QString& propertyId)
{
    if (propertyId.isEmpty()) return {};
    const auto it = namesById.find(propertyId);
    return it == namesById.end() ? propertyId : it.value();
}

} // namespace ui
