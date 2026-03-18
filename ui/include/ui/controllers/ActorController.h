/**
 * @file ui/include/ui/controllers/ActorController.h
 * @brief Declares the UI controller that forwards actor mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes actor create, update and delete actions to QML.
 */
class ActorController : public QObject {
    Q_OBJECT
public:
    explicit ActorController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addActor(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void updateActor(const QString& id, const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void deleteActor(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
