/**
 * @file ui/include/ui/controllers/PropertyController.h
 * @brief Declares the UI controller that forwards property mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes property create, update and delete actions to QML.
 */
class PropertyController : public QObject {
    Q_OBJECT
public:
    /** @brief Create a property controller bound to the application facade. */
    explicit PropertyController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single property by identifier. */
    Q_INVOKABLE QVariantMap property(const QString& id) const;

    /** @brief Return all properties. */
    Q_INVOKABLE QVariantList properties() const;

    /** @brief Create a new property from QML-provided values. */
    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description, const QStringList& aliases = {});

    /** @brief Update an existing property from QML-provided values. */
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description, const QStringList& aliases = {});

    /** @brief Delete a property by identifier. */
    Q_INVOKABLE void deleteProperty(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
