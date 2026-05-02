/**
 * @file ui/include/ui/controllers/ContractController.h
 * @brief Declares the UI controller that forwards contract mutations to the application facade.
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
 * @brief Exposes contract create, update and delete actions to QML.
 */
class ContractController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(ContractController)
    QML_UNCREATABLE("ContractController is provided by the application context")
public:
    /** @brief Create a contract controller bound to the application facade.
     *  @param core Core application facade pointer
     *  @param parent QObject parent
     */
    explicit ContractController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single contract by identifier.
     *  @param id Contract identifier
     *  @return Serialized contract payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap contract(const QString& id) const;

    /** @brief Return all contracts.
     *  @return List of serialized contracts as QVariantList
     */
    Q_INVOKABLE QVariantList contracts() const;

    /** @brief Create a new contract from QML-provided values.
     *  @param name Contract name
     *  @param type Contract type
     *  @param description Contract description
     *  @param actorIds Associated actor identifiers
     *  @param propertyIds Associated property identifiers
     *  @param aliases Optional aliases
     *  @return Identifier of the created contract
     */
    Q_INVOKABLE QString addContract(const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds,
                                    const QStringList& aliases = {});

    /** @brief Update an existing contract from QML-provided values.
     *  @param id Contract identifier
     *  @param name Contract name
     *  @param type Contract type
     *  @param description Contract description
     *  @param actorIds Associated actor identifiers
     *  @param propertyIds Associated property identifiers
     *  @param aliases Optional aliases
     */
    Q_INVOKABLE void updateContract(const QString& id, const QString& name, const QString& type, const QString& description,
                                    const QStringList& actorIds, const QStringList& propertyIds,
                                    const QStringList& aliases = {});
    Q_INVOKABLE QString saveContract(const QString& id,
                                     const QString& name,
                                     const QString& type,
                                     const QString& description,
                                     const QStringList& actorIds,
                                     const QStringList& propertyIds,
                                     const QStringList& aliases = {});

    /** @brief Delete a contract by identifier.
     *  @param id Contract identifier
     */
    Q_INVOKABLE void deleteContract(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
