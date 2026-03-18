/**
 * @file ui/include/ui/controllers/StatementController.h
 * @brief Declares the UI controller that forwards statement mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes statement create, update and delete actions to QML.
 */
class StatementController : public QObject {
    Q_OBJECT
public:
    explicit StatementController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addStatement(const QString& name);
    Q_INVOKABLE void updateStatement(const QString& id, const QString& name);
    Q_INVOKABLE void deleteStatement(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
