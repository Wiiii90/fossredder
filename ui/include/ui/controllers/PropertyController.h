/**
 * @file ui/include/ui/controllers/PropertyController.h
 * @brief Declares the UI controller that forwards property mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes property create, update and delete actions to QML.
 */
class PropertyController : public QObject {
    Q_OBJECT
public:
    explicit PropertyController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addProperty(const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void updateProperty(const QString& id, const QString& name, const QString& address, const QString& description);
    Q_INVOKABLE void deleteProperty(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
