/**
 * @file ui/include/ui/controllers/ActorController.h
 * @brief Declares the UI controller that forwards actor mutations to the application facade.
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
 * @brief Exposes actor create, update and delete actions to QML.
 */
class ActorController : public QObject {
    Q_OBJECT
public:
    /** @brief Create an actor controller bound to the application facade.
     *  @param core Core application facade pointer
     *  @param parent QObject parent
     */
    explicit ActorController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single actor by identifier. */
    Q_INVOKABLE QVariantMap actor(const QString& id) const;

    /** @brief Return all actors. */
    Q_INVOKABLE QVariantList actors() const;

    /** @brief Create a new actor from QML-provided values. */
    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});

    /** @brief Update an existing actor from QML-provided values. */
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description, const QStringList& aliases = {});

    /** @brief Delete an actor by identifier. */
    Q_INVOKABLE void deleteActor(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
