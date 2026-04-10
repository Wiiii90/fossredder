/**
 * @file ui/include/ui/state/PropertyNameProjection.h
 * @brief Declares helpers for property-name cache projection and lookup.
 */

#pragma once

#include <QHash>
#include <QString>

namespace ui {

class PropertyList;

QHash<QString, QString> buildPropertyNameIndex(const PropertyList& properties);
QString resolvePropertyName(const QHash<QString, QString>& namesById, const QString& propertyId);

} // namespace ui
