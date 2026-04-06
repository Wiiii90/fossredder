/**
 * @file ui/include/ui/state/PropertyNameCatalog.h
 * @brief Declarations for the UI PropertyNameCatalog component.
 */

#pragma once

#include <QHash>
#include <QString>

namespace ui {

class PropertyList;

class PropertyNameCatalog {
public:
    void rebuild(const PropertyList& properties);
    void remove(const QString& propertyId);
    QString name(const QString& propertyId) const;

private:
    QHash<QString, QString> namesById_;
};

}
