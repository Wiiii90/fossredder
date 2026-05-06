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
#include <qqmlintegration.h>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes property create, update and delete actions to QML.
 */
class PropertyController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PropertyController)
    QML_UNCREATABLE("PropertyController is provided by the application context")
public:
    /** @brief Create a property controller bound to the application facade. */
    explicit PropertyController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single property by identifier.
     *  @param id Property identifier
     *  @return Serialized property payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap property(const QString& id) const;

    /** @brief Return all properties.
     *  @return List of serialized properties as QVariantList
     */
    Q_INVOKABLE QVariantList properties() const;

    /** @brief Create a new property from QML-provided values.
     *  @param name Property name
     *  @param aliases Optional aliases
     *  @return Identifier of the created property
     */
    Q_INVOKABLE QString addProperty(const QString& name, const QStringList& aliases = {});

    /** @brief Update an existing property from QML-provided values.
     *  @param id Property identifier
     *  @param name Property name
     *  @param aliases Optional aliases
     */
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QStringList& aliases = {});

    Q_INVOKABLE QString saveProperty(const QString& id,
                                     const QString& name,
                                     const QStringList& aliases = {});

    /** @brief Delete a property by identifier.
     *  @param id Property identifier
     */
    Q_INVOKABLE void deleteProperty(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
