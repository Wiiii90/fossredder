/**
 * @file ui/include/ui/controllers/ContractController.h
 * @brief Declares the UI controller that forwards contract mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes contract create, update and delete actions to QML.
 */
class ContractController : public QObject {
    Q_OBJECT
public:
    explicit ContractController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds,
                                    const QStringList& aliases = {});
    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds,
                                    const QStringList& aliases = {});
    Q_INVOKABLE void deleteContract(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
